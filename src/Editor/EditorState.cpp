#include "EditorState.h"
#include <glfw3.h>
#include "engine/camera/camera.h"
#include "engine/io/input.h"
#include "engine/io/display.h"
#include "game/entities/EPlayer.h"
#include "editor/editor.h"
#include "engine/rvn.h"

REditorState::REditorState() = default;


bool REditorState::IsInGameMode()
{
	return Get()->current_mode == ProgramMode::Game;
}

bool REditorState::IsInEditorMode()
{
	return Get()->current_mode == ProgramMode::Editor;
}

bool REditorState::IsInConsoleMode()
{
	return Get()->current_mode == ProgramMode::Console;
}

void REditorState::ToggleProgramMode()
{
	auto* GII = GlobalInputInfo::Get();
	auto* GDC = GlobalDisplayState::Get();

	auto* player = EPlayer::Get();

	GII->forget_last_mouse_coords = true;
	auto* ES = Get();

	if (ES->current_mode == ProgramMode::Editor)
	{
		ES->last_mode = ES->current_mode;
		ES->current_mode = ProgramMode::Game;
		RCameraManager::Get()->SwitchToGameCamera();

		player->MakeInvisible();

		glfwSetInputMode(GDC->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		Editor::EndDearImguiFrame();

		Rvn::rm_buffer->Add("Game Mode", 2000);

	}

	else if (ES->current_mode == ProgramMode::Game)
	{
		ES->last_mode = ES->current_mode;
		ES->current_mode = ProgramMode::Editor;
		RCameraManager::Get()->SwitchToEditorCamera();

		player->MakeVisible();

		glfwSetInputMode(GDC->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Editor::StartDearImguiFrame();

		Rvn::rm_buffer->Add("Editor Mode", 2000);
	}
}
