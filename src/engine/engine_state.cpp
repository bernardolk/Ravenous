#include "engine_state.h"
#include <glfw3.h>

#include "camera/camera.h"
#include "engine/io/input.h"
#include "engine/io/display.h"
#include "game/entities/player.h"
#include "editor/editor.h"
#include "engine/rvn.h"

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

	if (ES->current_mode == ProgramMode::Editor)
	{
		ES->last_mode = ES->current_mode;
		ES->current_mode = ProgramMode::Game;
		CameraManager::Get()->SwitchToGameCamera();

		player->MakeInvisible();

		glfwSetInputMode(GDC->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		Editor::EndDearImguiFrame();

		Rvn::rm_buffer->Add("Game Mode", 2000);

	}

	else if (ES->current_mode == ProgramMode::Game)
	{
		ES->last_mode = ES->current_mode;
		ES->current_mode = ProgramMode::Editor;
		CameraManager::Get()->SwitchToEditorCamera();

		player->MakeVisible();

		glfwSetInputMode(GDC->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Editor::StartDearImguiFrame();

		Rvn::rm_buffer->Add("Editor Mode", 2000);
	}
}
