#include "editor/console/console.h"
#include "engine/world/scene.h"
#include "engine/camera/camera.h"
#include "engine/engine_state.h"
#include "engine/rvn.h"
#include "engine/io/display.h"
#include "engine/io/input.h"
#include "engine/io/input_phase.h"
#include "engine/render/text/text_renderer.h"
#include "engine/serialization/sr_config.h"
#include "engine/serialization/sr_world.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/world/scene_manager.h"
#include "game/entities/player.h"


void InitializeConsoleBuffers()
{
	auto buffers = static_cast<char**>(malloc(sizeof(char*) * Console.buffer_capacity));
	for (u16 i = 0; i < Console.buffer_capacity; i++)
	{
		buffers[i] = static_cast<char*>(calloc(Console.max_chars, sizeof(char)));
	}

	Console.buffers = buffers;
}

void RenderConsole()
{
	RenderText(15, GlobalDisplayConfig::viewport_height - 20, Console.scratch_buffer);
	RenderText(15, GlobalDisplayConfig::viewport_height - 35, std::to_string(Console.b_ind));
}

void MoveToNextBuffer()
{
	if (Console.b_ind < Console.current_buffer_size)
		Console.b_ind++;
	if (Console.b_ind < Console.current_buffer_size)
		CopyBufferToScratchBuffer();
	else
		ClearScratchBuffer();
}

void MoveToPreviousBuffer()
{
	if (Console.b_ind > 0)
		Console.b_ind--;
	if (Console.b_ind < Console.current_buffer_size)
		CopyBufferToScratchBuffer();
	else
		ClearScratchBuffer();
}

void CopyBufferToScratchBuffer()
{
	ClearScratchBuffer();

	int char_ind = 0;
	char scene_name[50] = {'\0'};
	while (Console.buffers[Console.b_ind][char_ind] != '\0')
	{
		Console.scratch_buffer[char_ind] = Console.buffers[Console.b_ind][char_ind];
		char_ind++;
	}

	Console.c_ind = char_ind;
}

void StartConsoleMode()
{
	auto* ES = EngineState::Get();
	ES->last_mode = ES->current_mode;
	ES->current_mode = EngineState::ProgramMode::Console;
}

void QuitConsoleMode()
{
	auto* ES = EngineState::Get();
	ES->current_mode = ES->last_mode;
	ES->last_mode = EngineState::ProgramMode::Console;
}

std::string CommitBuffer()
{
	// copy from scratch buffer to variable
	int char_ind = 0;
	char input[50] = {'\0'};
	while (Console.scratch_buffer[char_ind] != '\0')
	{
		input[char_ind] = Console.scratch_buffer[char_ind];
		char_ind++;
	}

	// realloc if necessary
	if (Console.current_buffer_size == Console.buffer_capacity)
	{
		auto prior_capacity = Console.buffer_capacity;
		Console.buffer_capacity *= 2;
		Console.buffers = static_cast<char**>(realloc(Console.buffers, sizeof(char*) * Console.buffer_capacity));
		for (u64 i = prior_capacity; i < Console.buffer_capacity; i++)
		{
			Console.buffers[i] = static_cast<char*>(calloc(Console.max_chars, sizeof(char)));
		}
	}

	// commit to buffers (log)
	char_ind = 0;
	while (Console.scratch_buffer[char_ind] != '\0')
	{
		Console.buffers[Console.current_buffer_size][char_ind] = Console.scratch_buffer[char_ind];
		char_ind++;
	}

	// clear scratch buffer
	ClearScratchBuffer();

	// updates number of items in buffers
	Console.current_buffer_size++;
	// move buffers pointer up the stack
	Console.b_ind = Console.current_buffer_size;

	return input;
}

void ClearScratchBuffer()
{
	int char_ind = 0;
	while (Console.scratch_buffer[char_ind] != '\0')
	{
		Console.scratch_buffer[char_ind] = '\0';
		char_ind++;
	}
	Console.c_ind = 0;
}

void ExecuteCommand(const std::string& buffer_line, Player* & player, T_World* world, Camera* camera)
{
	Parser p{buffer_line, 50};
	p.ParseToken();
	const std::string command = GetParsed<std::string>(p);
	auto* GSI = GlobalSceneInfo::Get();
	auto& program_config = *ProgramConfig::Get();

	// ---------------
	// 'SAVE' COMMAND
	// ---------------
	if (command == "save")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string argument = GetParsed<std::string>(p);
		WorldSerializer::SaveToFile(argument, false);
	}

	// ---------------
	// 'COPY' COMMAND
	// ---------------
	else if (command == "copy")
	{
		// if you dont want to switch to the new file when saving scene with a new name
		p.ParseWhitespace();
		p.ParseToken();
		const std::string scene_name = GetParsed<std::string>(p);
		WorldSerializer::SaveToFile(scene_name, true);
	}

	// ---------------
	// 'LOAD' COMMAND
	// ---------------
	else if (command == "load")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string scene_name = GetParsed<std::string>(p);
		// updates scene with new one
		if (WorldSerializer::LoadFromFile(scene_name))
		{
			player = GSI->player; // not irrelevant! do not delete
			if (EngineState::IsInEditorMode())
			{
				player->MakeVisible();
			}
			else
			{
				player->MakeInvisible();
			}

			ConfigSerializer::LoadGlobalConfigs();
			GSI->active_scene->ReloadGlobalConfigs();
		}
	}

	// --------------
	// 'NEW' COMMAND
	// --------------
	else if (command == "new")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string scene_name = GetParsed<std::string>(p);
		if (scene_name != "")
		{
			auto current_scene = GSI->scene_name;
			if (WorldSerializer::CheckIfSceneExists(scene_name))
			{
				Rvn::rm_buffer->Add("Scene name already exists.", 3000);
				return;
			}

			if (!WorldSerializer::LoadFromFile(Paths::SceneTemplate))
			{
				Rvn::rm_buffer->Add("Scene template not found.", 3000);
				return;
			}

			if (!WorldSerializer::SaveToFile(scene_name, false))
			{
				// if couldnt save copy of template, falls back, so we dont edit the template by mistake
				if (WorldSerializer::LoadFromFile(current_scene))
				{
					assert(false); // if this happens, weird!
				}

				Rvn::rm_buffer->Add("Couldnt save new scene.", 3000);
			}

			player = GSI->player; // not irrelevant! do not delete
			if (EngineState::IsInEditorMode())
				player->flags &= ~EntityFlags_InvisibleEntity;
			else
				player->flags |= EntityFlags_InvisibleEntity;
		}
		else
		{
			Rvn::rm_buffer->Add("Could not create new scene. Provide a name please.", 3000);
		}
	}

	// --------------
	// 'SET' COMMAND
	// --------------

	else if (command == "set")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string argument = GetParsed<std::string>(p);
		if (argument == "scene")
		{
			program_config.initial_scene = GSI->scene_name;
			ConfigSerializer::Save(program_config);
		}
		else if (argument == "all")
		{
			// save scene
			player->checkpoint_pos = player->position;
			WorldSerializer::SaveToFile();
			// set scene
			program_config.initial_scene = GSI->scene_name;
			ConfigSerializer::Save(program_config);
		}
		else
			print("you can set 'scene' or 'all'. dude. %s  won't work.", command.c_str());
	}

	// -----------------
	// 'RELOAD' COMMAND
	// -----------------
	else if (command == "reload")
	{
		if (WorldSerializer::LoadFromFile(GSI->scene_name))
		{
			player = GSI->player; // not irrelevant! do not delete

			if (EngineState::IsInEditorMode())
				player->flags &= ~EntityFlags_InvisibleEntity;
			else
				player->flags |= EntityFlags_InvisibleEntity;

			ConfigSerializer::LoadGlobalConfigs();
			GSI->active_scene->ReloadGlobalConfigs();
			GlobalInputInfo::Get()->block_mouse_move = false;
		}
	}

	// ----------------
	// 'LIVES' COMMAND
	// ----------------
	else if (command == "lives")
	{
		player->RestoreHealth();
	}

	// ---------------
	// 'KILL' COMMAND
	// ---------------
	else if (command == "kill")
	{
		player->Die();
	}

	// ---------------
	// 'MOVE' COMMAND
	// ---------------
	else if (command == "move")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string argument = GetParsed<std::string>(p);
		if (argument == "cam")
		{
			p.ParseVec3();
			camera->position = GetParsed<vec3>(p);
		}
		else
			print("you can move cam only at the moment dude. I don't know what %s %s means man.", command.c_str(), argument.c_str());
	}
	else
		print("what do you mean with %s man?\n", command.c_str());
}

void HandleConsoleInput(InputFlags flags, Player* & player, T_World* world, Camera* camera)
{
	if (PressedOnce(flags, KEY_ENTER))
	{
		// if empty, just quit
		if (Console.scratch_buffer[0] == '\0')
		{
			QuitConsoleMode();
			return;
		}
		const auto buffer_line = CommitBuffer();
		ExecuteCommand(buffer_line, player, world, camera);
		QuitConsoleMode();
	}

	if (PressedOnce(flags, KEY_GRAVE_TICK))
	{
		QuitConsoleMode();
	}

	if (PressedOnce(flags, KEY_UP))
	{
		MoveToPreviousBuffer();
	}

	if (PressedOnce(flags, KEY_DOWN))
	{
		MoveToNextBuffer();
	}

	// run through all letters to see if they were hit
	CheckLetterKeyPresses(flags);
}

void CheckLetterKeyPresses(InputFlags flags)
{
	if (PressedOnce(flags, KEY_BACKSPACE))
	{
		if (Console.c_ind > 0)
			Console.scratch_buffer[--Console.c_ind] = '\0';
	}
	if (PressedOnce(flags, KEY_Q))
	{
		Console.scratch_buffer[Console.c_ind++] = 'q';
	}
	if (PressedOnce(flags, KEY_W))
	{
		Console.scratch_buffer[Console.c_ind++] = 'w';
	}
	if (PressedOnce(flags, KEY_E))
	{
		Console.scratch_buffer[Console.c_ind++] = 'e';
	}
	if (PressedOnce(flags, KEY_R))
	{
		Console.scratch_buffer[Console.c_ind++] = 'r';
	}
	if (PressedOnce(flags, KEY_T))
	{
		Console.scratch_buffer[Console.c_ind++] = 't';
	}
	if (PressedOnce(flags, KEY_Y))
	{
		Console.scratch_buffer[Console.c_ind++] = 'y';
	}
	if (PressedOnce(flags, KEY_U))
	{
		Console.scratch_buffer[Console.c_ind++] = 'u';
	}
	if (PressedOnce(flags, KEY_I))
	{
		Console.scratch_buffer[Console.c_ind++] = 'i';
	}
	if (PressedOnce(flags, KEY_O))
	{
		Console.scratch_buffer[Console.c_ind++] = 'o';
	}
	if (PressedOnce(flags, KEY_P))
	{
		Console.scratch_buffer[Console.c_ind++] = 'p';
	}
	if (PressedOnce(flags, KEY_A))
	{
		Console.scratch_buffer[Console.c_ind++] = 'a';
	}
	if (PressedOnce(flags, KEY_S))
	{
		Console.scratch_buffer[Console.c_ind++] = 's';
	}
	if (PressedOnce(flags, KEY_D))
	{
		Console.scratch_buffer[Console.c_ind++] = 'd';
	}
	if (PressedOnce(flags, KEY_F))
	{
		Console.scratch_buffer[Console.c_ind++] = 'f';
	}
	if (PressedOnce(flags, KEY_G))
	{
		Console.scratch_buffer[Console.c_ind++] = 'g';
	}
	if (PressedOnce(flags, KEY_H))
	{
		Console.scratch_buffer[Console.c_ind++] = 'h';
	}
	if (PressedOnce(flags, KEY_J))
	{
		Console.scratch_buffer[Console.c_ind++] = 'j';
	}
	if (PressedOnce(flags, KEY_K))
	{
		Console.scratch_buffer[Console.c_ind++] = 'k';
	}
	if (PressedOnce(flags, KEY_L))
	{
		Console.scratch_buffer[Console.c_ind++] = 'l';
	}
	if (PressedOnce(flags, KEY_Z))
	{
		Console.scratch_buffer[Console.c_ind++] = 'z';
	}
	if (PressedOnce(flags, KEY_X))
	{
		Console.scratch_buffer[Console.c_ind++] = 'x';
	}
	if (PressedOnce(flags, KEY_C))
	{
		Console.scratch_buffer[Console.c_ind++] = 'c';
	}
	if (PressedOnce(flags, KEY_V))
	{
		Console.scratch_buffer[Console.c_ind++] = 'v';
	}
	if (PressedOnce(flags, KEY_B))
	{
		Console.scratch_buffer[Console.c_ind++] = 'b';
	}
	if (PressedOnce(flags, KEY_N))
	{
		Console.scratch_buffer[Console.c_ind++] = 'n';
	}
	if (PressedOnce(flags, KEY_M))
	{
		Console.scratch_buffer[Console.c_ind++] = 'm';
	}
	if (PressedOnce(flags, KEY_1))
	{
		Console.scratch_buffer[Console.c_ind++] = '1';
	}
	if (PressedOnce(flags, KEY_2))
	{
		Console.scratch_buffer[Console.c_ind++] = '2';
	}
	if (PressedOnce(flags, KEY_3))
	{
		Console.scratch_buffer[Console.c_ind++] = '3';
	}
	if (PressedOnce(flags, KEY_4))
	{
		Console.scratch_buffer[Console.c_ind++] = '4';
	}
	if (PressedOnce(flags, KEY_5))
	{
		Console.scratch_buffer[Console.c_ind++] = '5';
	}
	if (PressedOnce(flags, KEY_6))
	{
		Console.scratch_buffer[Console.c_ind++] = '6';
	}
	if (PressedOnce(flags, KEY_7))
	{
		Console.scratch_buffer[Console.c_ind++] = '7';
	}
	if (PressedOnce(flags, KEY_8))
	{
		Console.scratch_buffer[Console.c_ind++] = '8';
	}
	if (PressedOnce(flags, KEY_9))
	{
		Console.scratch_buffer[Console.c_ind++] = '9';
	}
	if (PressedOnce(flags, KEY_0))
	{
		Console.scratch_buffer[Console.c_ind++] = '0';
	}
	if (PressedOnce(flags, KEY_SPACE))
	{
		Console.scratch_buffer[Console.c_ind++] = ' ';
	}
}
