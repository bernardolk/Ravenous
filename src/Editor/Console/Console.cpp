#include "editor/console/console.h"
#include "engine/camera/camera.h"
#include "editor/EditorState.h"
#include "engine/rvn.h"
#include "engine/io/display.h"
#include "engine/io/input.h"
#include "engine/io/InputPhase.h"
#include "engine/render/text/TextRenderer.h"
#include "engine/serialization/sr_config.h"
#include "engine/serialization/sr_world.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/world/World.h"
#include "game/entities/EPlayer.h"

RGlobalConsoleState::RGlobalConsoleState() = default;

void InitializeConsoleBuffers()
{
	auto& Console = *RGlobalConsoleState::Get();
	auto Buffers = static_cast<char**>(malloc(sizeof(char*) * Console.buffer_capacity));
	for (uint16 I = 0; I < Console.buffer_capacity; I++)
	{
		buffers[I] = static_cast<char*>(calloc(Console.max_chars, sizeof(char)));
	}

	Console.buffers = buffers;
}

void RenderConsole()
{
	auto& Console = *RGlobalConsoleState::Get();
	RenderText(15, GlobalDisplayState::viewport_height - 20, Console.scratch_buffer);
	RenderText(15, GlobalDisplayState::viewport_height - 35, std::to_string(Console.b_ind));
}

void MoveToNextBuffer()
{
	auto& Console = *RGlobalConsoleState::Get();
	if (Console.b_ind < Console.current_buffer_size)
		Console.b_ind++;
	if (Console.b_ind < Console.current_buffer_size)
		CopyBufferToScratchBuffer();
	else
		ClearScratchBuffer();
}

void MoveToPreviousBuffer()
{
	auto& Console = *RGlobalConsoleState::Get();
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
	auto& Console = *RGlobalConsoleState::Get();

	int CharInd = 0;
	char SceneName[50] = {'\0'};
	while (Console.buffers[Console.b_ind][CharInd] != '\0')
	{
		Console.scratch_buffer[CharInd] = Console.buffers[Console.b_ind][CharInd];
		CharInd++;
	}

	Console.c_ind = CharInd;
}

void StartConsoleMode()
{
	auto* ES = REditorState::Get();
	ES->LastMode = ES->CurrentMode;
	ES->CurrentMode = REditorState::ProgramMode::Console;
}

void QuitConsoleMode()
{
	auto* ES = REditorState::Get();
	ES->CurrentMode = ES->LastMode;
	ES->LastMode = REditorState::ProgramMode::Console;
}

std::string CommitBuffer()
{
	auto& Console = *RGlobalConsoleState::Get();

	// copy from scratch buffer to variable
	int CharInd = 0;
	char Input[50] = {'\0'};
	while (Console.scratch_buffer[CharInd] != '\0')
	{
		Input[CharInd] = Console.scratch_buffer[CharInd];
		CharInd++;
	}

	// realloc if necessary
	if (Console.current_buffer_size == Console.buffer_capacity)
	{
		auto PriorCapacity = Console.buffer_capacity;
		Console.buffer_capacity *= 2;
		Console.buffers = static_cast<char**>(realloc(Console.buffers, sizeof(char*) * Console.buffer_capacity));
		for (uint64 I = PriorCapacity; I < Console.buffer_capacity; I++)
		{
			Console.buffers[I] = static_cast<char*>(calloc(Console.max_chars, sizeof(char)));
		}
	}

	// commit to buffers (log)
	CharInd = 0;
	while (Console.scratch_buffer[CharInd] != '\0')
	{
		Console.buffers[Console.current_buffer_size][CharInd] = Console.scratch_buffer[CharInd];
		CharInd++;
	}

	// clear scratch buffer
	ClearScratchBuffer();

	// updates number of items in buffers
	Console.current_buffer_size++;
	// move buffers pointer up the stack
	Console.b_ind = Console.current_buffer_size;

	return Input;
}

void ClearScratchBuffer()
{
	auto& Console = *RGlobalConsoleState::Get();

	int CharInd = 0;
	while (Console.scratch_buffer[CharInd] != '\0')
	{
		Console.scratch_buffer[CharInd] = '\0';
		CharInd++;
	}
	Console.c_ind = 0;
}

void ExecuteCommand(const std::string& BufferLine, EPlayer* & Player, RWorld* World, RCamera* Camera)
{
	Parser P{buffer_line, 50};
	P.ParseToken();
	const std::string Command = GetParsed<std::string>(p);
	auto& ProgramConfig = *ProgramConfig::Get();

	// ---------------
	// 'SAVE' COMMAND
	// ---------------
	if (command == "save")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const std::string Argument = GetParsed<std::string>(p);
		WorldSerializer::SaveToFile(argument, false);
	}

	// ---------------
	// 'COPY' COMMAND
	// ---------------
	else if (command == "copy")
	{
		// if you dont want to switch to the new file when saving scene with a new name
		P.ParseWhitespace();
		P.ParseToken();
		const std::string SceneName = GetParsed<std::string>(p);
		WorldSerializer::SaveToFile(scene_name, true);
	}

	// ---------------
	// 'LOAD' COMMAND
	// ---------------
	else if (command == "load")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const std::string SceneName = GetParsed<std::string>(p);
		// updates scene with new one
		if (WorldSerializer::LoadFromFile(scene_name))
		{
			if (REditorState::IsInEditorMode())
			{
				Player->MakeVisible();
			}
			else
			{
				Player->MakeInvisible();
			}

			ConfigSerializer::LoadGlobalConfigs();
		}
	}

	// --------------
	// 'NEW' COMMAND
	// --------------
	else if (command == "new")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const std::string SceneName = GetParsed<std::string>(p);
		if (scene_name != "")
		{
			auto CurrentScene = RWorld::Get()->SceneName;
			if (WorldSerializer::CheckIfSceneExists(scene_name))
			{
				Rvn::RmBuffer->Add("Scene name already exists.", 3000);
				return;
			}

			if (!WorldSerializer::LoadFromFile(Paths::SceneTemplate))
			{
				Rvn::RmBuffer->Add("Scene template not found.", 3000);
				return;
			}

			if (!WorldSerializer::SaveToFile(scene_name, false))
			{
				// if couldnt save copy of template, falls back, so we dont edit the template by mistake
				if (WorldSerializer::LoadFromFile(current_scene))
				{
					assert(false); // if this happens, weird!
				}

				Rvn::RmBuffer->Add("Couldnt save new scene.", 3000);
			}

			if (REditorState::IsInEditorMode())
				Player->Flags &= ~EntityFlags_InvisibleEntity;
			else
				Player->Flags |= EntityFlags_InvisibleEntity;
		}
		else
		{
			Rvn::RmBuffer->Add("Could not create new scene. Provide a name please.", 3000);
		}
	}

	// --------------
	// 'SET' COMMAND
	// --------------

	else if (command == "set")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const std::string Argument = GetParsed<std::string>(p);
		if (argument == "scene")
		{
			ProgramConfig.InitialScene = RWorld::Get()->SceneName;
			ConfigSerializer::Save(ProgramConfig);
		}
		else if (argument == "all")
		{
			// save scene
			Player->checkpoint_pos = Player->Position;
			WorldSerializer::SaveToFile();
			// set scene
			ProgramConfig.InitialScene = RWorld::Get()->SceneName;
			ConfigSerializer::Save(ProgramConfig);
		}
		else
			print("you can set 'scene' or 'all'. dude. %s  won't work.", command.c_str());
	}

	// -----------------
	// 'RELOAD' COMMAND
	// -----------------
	else if (command == "reload")
	{
		if (WorldSerializer::LoadFromFile(RWorld::Get()->SceneName))
		{
			if (REditorState::IsInEditorMode())
				Player->Flags &= ~EntityFlags_InvisibleEntity;
			else
				Player->Flags |= EntityFlags_InvisibleEntity;

			ConfigSerializer::LoadGlobalConfigs();
			GlobalInputInfo::Get()->BlockMouseMove = false;
		}
	}

	// ----------------
	// 'LIVES' COMMAND
	// ----------------
	else if (command == "lives")
	{
		Player->RestoreHealth();
	}

	// ---------------
	// 'KILL' COMMAND
	// ---------------
	else if (command == "kill")
	{
		Player->Die();
	}

	// ---------------
	// 'MOVE' COMMAND
	// ---------------
	else if (command == "move")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const std::string Argument = GetParsed<std::string>(p);
		if (argument == "cam")
		{
			P.ParseVec3();
			camera->position = GetParsed<vec3>(p);
		}
		else
			print("you can move cam only at the moment dude. I don't know what %s %s means man.", command.c_str(), argument.c_str());
	}
	else
		print("what do you mean with %s man?\n", command.c_str());
}

void HandleConsoleInput(RInputFlags Flags, EPlayer* & Player, RWorld* World, RCamera* Camera)
{
	auto& Console = *RGlobalConsoleState::Get();

	if (PressedOnce(flags, KEY_ENTER))
	{
		// if empty, just quit
		if (Console.scratch_buffer[0] == '\0')
		{
			QuitConsoleMode();
			return;
		}
		const auto BufferLine = CommitBuffer();
		ExecuteCommand(buffer_line, Player, World, Camera);
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
	CheckLetterKeyPresses(Flags);
}

void CheckLetterKeyPresses(RInputFlags Flags)
{
	auto& Console = *RGlobalConsoleState::Get();

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
