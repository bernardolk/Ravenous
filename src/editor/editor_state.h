#pragma once

#include "engine/core/core.h"


struct EditorState
{
	DeclSingleton(EditorState)
	
	enum class ProgramMode : u8
	{
		Game,
		Editor,
		Console
	};

	ProgramMode current_mode = ProgramMode::Editor;
	ProgramMode last_mode = ProgramMode::Editor;

public:
	static bool IsInGameMode();
	static bool IsInEditorMode();
	static bool IsInConsoleMode();
	static void ToggleProgramMode();
};
