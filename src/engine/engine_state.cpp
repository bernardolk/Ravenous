#include "engine_state.h"

#include <glfw3.h>

#include "engine/io/input.h"
#include "engine/io/display.h"
#include "player.h"
#include "engine/world/scene_manager.h"
#include "editor/editor_interface.h"
#include "rvn.h"

bool EngineState::IsInGameMode()
{
	return Get()->current_mode == ProgramMode::Game;
}

bool EngineState::IsInEditorMode()
{
	return Get()->current_mode == ProgramMode::Editor;
}

bool EngineState::IsInConsoleMode()
{
	return Get()->current_mode == ProgramMode::Console;
}


void EngineState::ToggleProgramMode()
{
	auto* GII = GlobalInputInfo::Get();
	auto* GDC = GlobalDisplayConfig::Get();

	auto* player = Player::Get();

	GII->forget_last_mouse_coords = true;
	auto* ES = Get();
	auto* GSI = GlobalSceneInfo::Get();

	if(ES->current_mode == ProgramMode::Editor)
	{
		ES->last_mode = ES->current_mode;
		ES->current_mode = ProgramMode::Game;
		GSI->camera = GSI->views[1];

		player->MakeInvisible();

		glfwSetInputMode(GDC->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		Editor::end_dear_imgui_frame();

		Rvn::rm_buffer->Add("Game Mode", 2000);
	}

	else if(ES->current_mode == ProgramMode::Game)
	{
		ES->last_mode = ES->current_mode;
		ES->current_mode = ProgramMode::Editor;
		GSI->camera = GSI->views[0];

		player->MakeVisible();

		glfwSetInputMode(GDC->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Editor::start_dear_imgui_frame();

		Rvn::rm_buffer->Add("Editor Mode", 2000);
	}
}