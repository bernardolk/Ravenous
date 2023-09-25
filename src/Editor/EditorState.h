#pragma once

#include "engine/core/core.h"


struct REditorState
{
	DeclSingleton(REditorState)
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
