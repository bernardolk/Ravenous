#include "editor/console/console.h"
#include "engine/camera/camera.h"
#include "editor/EditorState.h"
#include "Editor/Reflection/Serialization.h"
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

void InitializeConsoleBuffers()
{
	auto& Console = *RGlobalConsoleState::Get();
	auto Buffers = static_cast<char**>(malloc(sizeof(char*) * Console.buffer_capacity));
	for (uint16 i = 0; i < Console.buffer_capacity; i++)
	{
		Buffers[i] = static_cast<char*>(calloc(Console.max_chars, sizeof(char)));
	}

	Console.buffers = Buffers;
}

void RenderConsole()
{
	auto& Console = *RGlobalConsoleState::Get();
	RenderText(15, GlobalDisplayState::ViewportHeight - 20, Console.scratch_buffer);
	RenderText(15, GlobalDisplayState::ViewportHeight - 35, std::to_string(Console.b_ind));
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

string CommitBuffer()
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

void ExecuteCommand(const string& BufferLine, EPlayer* & Player, RWorld* World, RCamera* Camera)
{
	Parser P{BufferLine, 50};
	P.ParseToken();
	const string Command = GetParsed<string>(P);
	auto& ProgramConfig = *ProgramConfig::Get();

	// ---------------
	// 'SAVE' COMMAND
	// ---------------
	if (Command == "save")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const string Argument = GetParsed<string>(P);
		WorldSerializer::SaveToFile(Argument, false);
	}

	// ---------------
	// 'COPY' COMMAND
	// ---------------
	else if (Command == "copy")
	{
		// if you dont want to switch to the new file when saving scene with a new name
		P.ParseWhitespace();
		P.ParseToken();
		const string SceneName = GetParsed<string>(P);
		WorldSerializer::SaveToFile(SceneName, true);
	}

	// ---------------
	// 'LOAD' COMMAND
	// ---------------
	else if (Command == "load")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const string SceneName = GetParsed<string>(P);
		// updates scene with new one
		if (WorldSerializer::LoadFromFile(SceneName))
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
	else if (Command == "new")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const string SceneName = GetParsed<string>(P);
		if (SceneName != "")
		{
			auto CurrentScene = RWorld::Get()->SceneName;
			if (WorldSerializer::CheckIfSceneExists(SceneName))
			{
				Rvn::RmBuffer->Add("Scene name already exists.", 3000);
				return;
			}

			if (!WorldSerializer::LoadFromFile(Paths::SceneTemplate))
			{
				Rvn::RmBuffer->Add("Scene template not found.", 3000);
				return;
			}

			if (!WorldSerializer::SaveToFile(SceneName, false))
			{
				// if couldnt save copy of template, falls back, so we dont edit the template by mistake
				if (WorldSerializer::LoadFromFile(CurrentScene))
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

	else if (Command == "set")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const string Argument = GetParsed<string>(P);
		if (Argument == "scene")
		{
			ProgramConfig.InitialScene = RWorld::Get()->SceneName;
			ConfigSerializer::Save(ProgramConfig);
		}
		else if (Argument == "all")
		{
			// save scene
			Player->CheckpointPos = Player->Position;
			WorldSerializer::SaveToFile();
			// set scene
			ProgramConfig.InitialScene = RWorld::Get()->SceneName;
			ConfigSerializer::Save(ProgramConfig);
		}
	}

	// -----------------
	// 'RELOAD' COMMAND
	// -----------------
	else if (Command == "reload")
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
	else if (Command == "lives")
	{
		Player->RestoreHealth();
	}

	// ---------------
	// 'KILL' COMMAND
	// ---------------
	else if (Command == "kill")
	{
		Player->Die();
	}

	// ---------------
	// 'MOVE' COMMAND
	// ---------------
	else if (Command == "move")
	{
		P.ParseWhitespace();
		P.ParseToken();
		const string Argument = GetParsed<string>(P);
		if (Argument == "cam")
		{
			P.ParseVec3();
			Camera->Position = GetParsed<vec3>(P);
		}
	}

	else if (Command == "testsave")
	{
		Serialization::SaveWorldToDisk();
	}
	
	else {
		Log("Console command not understood: \"%s\"\n", Command.c_str());
	}
}

void HandleConsoleInput(RInputFlags Flags, EPlayer* & Player, RWorld* World, RCamera* Camera)
{
	auto& Console = *RGlobalConsoleState::Get();

	if (PressedOnce(Flags, NKeyInput::KeyEnter))
	{
		// if empty, just quit
		if (Console.scratch_buffer[0] == '\0')
		{
			QuitConsoleMode();
			return;
		}
		const auto BufferLine = CommitBuffer();
		ExecuteCommand(BufferLine, Player, World, Camera);
		QuitConsoleMode();
	}

	if (PressedOnce(Flags, NKeyInput::KeyGraveTick))
	{
		QuitConsoleMode();
	}

	if (PressedOnce(Flags, NKeyInput::KeyUp))
	{
		MoveToPreviousBuffer();
	}

	if (PressedOnce(Flags, NKeyInput::KeyDown))
	{
		MoveToNextBuffer();
	}

	// run through all letters to see if they were hit
	CheckLetterKeyPresses(Flags);
}

void CheckLetterKeyPresses(RInputFlags Flags)
{
	auto& Console = *RGlobalConsoleState::Get();

	if (PressedOnce(Flags, NKeyInput::KeyBackspace))
	{
		if (Console.c_ind > 0)
			Console.scratch_buffer[--Console.c_ind] = '\0';
	}
	if (PressedOnce(Flags, NKeyInput::KeyQ))
	{
		Console.scratch_buffer[Console.c_ind++] = 'q';
	}
	if (PressedOnce(Flags, NKeyInput::KeyW))
	{
		Console.scratch_buffer[Console.c_ind++] = 'w';
	}
	if (PressedOnce(Flags, NKeyInput::KeyE))
	{
		Console.scratch_buffer[Console.c_ind++] = 'e';
	}
	if (PressedOnce(Flags, NKeyInput::KeyR))
	{
		Console.scratch_buffer[Console.c_ind++] = 'r';
	}
	if (PressedOnce(Flags, NKeyInput::KeyT))
	{
		Console.scratch_buffer[Console.c_ind++] = 't';
	}
	if (PressedOnce(Flags, NKeyInput::KeyY))
	{
		Console.scratch_buffer[Console.c_ind++] = 'y';
	}
	if (PressedOnce(Flags, NKeyInput::KeyU))
	{
		Console.scratch_buffer[Console.c_ind++] = 'u';
	}
	if (PressedOnce(Flags, NKeyInput::KeyI))
	{
		Console.scratch_buffer[Console.c_ind++] = 'i';
	}
	if (PressedOnce(Flags, NKeyInput::KeyO))
	{
		Console.scratch_buffer[Console.c_ind++] = 'o';
	}
	if (PressedOnce(Flags, NKeyInput::KeyP))
	{
		Console.scratch_buffer[Console.c_ind++] = 'p';
	}
	if (PressedOnce(Flags, NKeyInput::KeyA))
	{
		Console.scratch_buffer[Console.c_ind++] = 'a';
	}
	if (PressedOnce(Flags, NKeyInput::KeyS))
	{
		Console.scratch_buffer[Console.c_ind++] = 's';
	}
	if (PressedOnce(Flags, NKeyInput::KeyD))
	{
		Console.scratch_buffer[Console.c_ind++] = 'd';
	}
	if (PressedOnce(Flags, NKeyInput::KeyF))
	{
		Console.scratch_buffer[Console.c_ind++] = 'f';
	}
	if (PressedOnce(Flags, NKeyInput::KeyG))
	{
		Console.scratch_buffer[Console.c_ind++] = 'g';
	}
	if (PressedOnce(Flags, NKeyInput::KeyH))
	{
		Console.scratch_buffer[Console.c_ind++] = 'h';
	}
	if (PressedOnce(Flags, NKeyInput::KeyJ))
	{
		Console.scratch_buffer[Console.c_ind++] = 'j';
	}
	if (PressedOnce(Flags, NKeyInput::KeyK))
	{
		Console.scratch_buffer[Console.c_ind++] = 'k';
	}
	if (PressedOnce(Flags, NKeyInput::KeyL))
	{
		Console.scratch_buffer[Console.c_ind++] = 'l';
	}
	if (PressedOnce(Flags, NKeyInput::KeyZ))
	{
		Console.scratch_buffer[Console.c_ind++] = 'z';
	}
	if (PressedOnce(Flags, NKeyInput::KeyX))
	{
		Console.scratch_buffer[Console.c_ind++] = 'x';
	}
	if (PressedOnce(Flags, NKeyInput::KeyC))
	{
		Console.scratch_buffer[Console.c_ind++] = 'c';
	}
	if (PressedOnce(Flags, NKeyInput::KeyV))
	{
		Console.scratch_buffer[Console.c_ind++] = 'v';
	}
	if (PressedOnce(Flags, NKeyInput::KeyB))
	{
		Console.scratch_buffer[Console.c_ind++] = 'b';
	}
	if (PressedOnce(Flags, NKeyInput::KeyN))
	{
		Console.scratch_buffer[Console.c_ind++] = 'n';
	}
	if (PressedOnce(Flags, NKeyInput::KeyM))
	{
		Console.scratch_buffer[Console.c_ind++] = 'm';
	}
	if (PressedOnce(Flags, NKeyInput::Key1))
	{
		Console.scratch_buffer[Console.c_ind++] = '1';
	}
	if (PressedOnce(Flags, NKeyInput::Key2))
	{
		Console.scratch_buffer[Console.c_ind++] = '2';
	}
	if (PressedOnce(Flags, NKeyInput::Key3))
	{
		Console.scratch_buffer[Console.c_ind++] = '3';
	}
	if (PressedOnce(Flags, NKeyInput::Key4))
	{
		Console.scratch_buffer[Console.c_ind++] = '4';
	}
	if (PressedOnce(Flags, NKeyInput::Key5))
	{
		Console.scratch_buffer[Console.c_ind++] = '5';
	}
	if (PressedOnce(Flags, NKeyInput::Key6))
	{
		Console.scratch_buffer[Console.c_ind++] = '6';
	}
	if (PressedOnce(Flags, NKeyInput::Key7))
	{
		Console.scratch_buffer[Console.c_ind++] = '7';
	}
	if (PressedOnce(Flags, NKeyInput::Key8))
	{
		Console.scratch_buffer[Console.c_ind++] = '8';
	}
	if (PressedOnce(Flags, NKeyInput::Key9))
	{
		Console.scratch_buffer[Console.c_ind++] = '9';
	}
	if (PressedOnce(Flags, NKeyInput::Key0))
	{
		Console.scratch_buffer[Console.c_ind++] = '0';
	}
	if (PressedOnce(Flags, NKeyInput::KeySpace))
	{
		Console.scratch_buffer[Console.c_ind++] = ' ';
	}
}
