#pragma once

#include "engine/core/core.h"


struct REditorState
{
	static REditorState* Get() 
	{ 
		static REditorState Instance{};
		return &Instance;
	}
	
	enum class ProgramMode : uint8
	{
		Game,
		Editor,
		Console
	};

	ProgramMode CurrentMode = ProgramMode::Editor;
	ProgramMode LastMode = ProgramMode::Editor;

	static bool IsInGameMode();
	static bool IsInEditorMode();
	static bool IsInConsoleMode();
	static void ToggleProgramMode();
};
