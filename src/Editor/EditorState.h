#pragma once

#include "engine/core/core.h"


struct REditorState
{
	static REditorState* Get() 
	{ 
		static REditorState Instance{};
		return &Instance;
	}
	
	enum class NProgramMode : uint8
	{
		Game,
		Editor,
		Console
	};

	NProgramMode CurrentMode = NProgramMode::Editor;
	NProgramMode LastMode = NProgramMode::Editor;

	static bool IsInGameMode();
	static bool IsInEditorMode();
	static bool IsInConsoleMode();
	static void ToggleProgramMode();
};
