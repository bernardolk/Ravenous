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
	return Get()->CurrentMode == ProgramMode::Game;
}

bool REditorState::IsInEditorMode()
{
	return Get()->CurrentMode == ProgramMode::Editor;
}

bool REditorState::IsInConsoleMode()
{
	return Get()->CurrentMode == ProgramMode::Console;
}

void REditorState::ToggleProgramMode()
{
	auto* GII = GlobalInputInfo::Get();
	auto* GDC = GlobalDisplayState::Get();

	auto* Player = EPlayer::Get();

	GII->ForgetLastMouseCoords = true;
	auto* ES = Get();

	if (ES->CurrentMode == ProgramMode::Editor)
	{
		ES->LastMode = ES->CurrentMode;
		ES->CurrentMode = ProgramMode::Game;
		RCameraManager::Get()->SwitchToGameCamera();

		Player->MakeInvisible();

		glfwSetInputMode(GDC->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		Editor::EndDearImguiFrame();

		Rvn::RmBuffer->Add("Game Mode", 2000);

	}

	else if (ES->CurrentMode == ProgramMode::Game)
	{
		ES->LastMode = ES->CurrentMode;
		ES->CurrentMode = ProgramMode::Editor;
		RCameraManager::Get()->SwitchToEditorCamera();

		Player->MakeVisible();

		glfwSetInputMode(GDC->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Editor::StartDearImguiFrame();

		Rvn::RmBuffer->Add("Editor Mode", 2000);
	}
}
