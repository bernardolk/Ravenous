#include "EditorState.h"
#include <glfw3.h>
#include "engine/camera/camera.h"
#include "engine/io/input.h"
#include "engine/io/display.h"
#include "..\Game\Entities\Player.h"
#include "Editor/EditorMain.h"
#include "engine/rvn.h"


bool REditorState::IsInGameMode()
{
	return Get()->CurrentMode == NProgramMode::Game;
}

bool REditorState::IsInEditorMode()
{
	return Get()->CurrentMode == NProgramMode::Editor;
}

bool REditorState::IsInConsoleMode()
{
	return Get()->CurrentMode == NProgramMode::Console;
}

void REditorState::ToggleProgramMode()
{
	auto* GII = GlobalInputInfo::Get();
	auto* GDC = GlobalDisplayState::Get();

	auto* Player = EPlayer::Get();

	GII->ForgetLastMouseCoords = true;
	auto* ES = Get();

	if (ES->CurrentMode == NProgramMode::Editor)
	{
		ES->LastMode = ES->CurrentMode;
		ES->CurrentMode = NProgramMode::Game;
		RCameraManager::Get()->SwitchToGameCamera();

		Player->MakeInvisible();

		glfwSetInputMode(GDC->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		Editor::EndDearImguiFrame();

		PrintEditorMsg("Game Mode");

	}

	else if (ES->CurrentMode == NProgramMode::Game)
	{
		ES->LastMode = ES->CurrentMode;
		ES->CurrentMode = NProgramMode::Editor;
		RCameraManager::Get()->SwitchToEditorCamera();

		Player->MakeVisible();

		glfwSetInputMode(GDC->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Editor::StartDearImguiFrame();

		PrintEditorMsg("Editor Mode");
	}
}
