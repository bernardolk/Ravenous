#include "engine/MainLoop.h"

#include <glfw3.h>
#include <imgui.h>

#include "RavenousEngine.h"
#include "game/animation/AnPlayer.h"
#include "game/animation/AnUpdate.h"
#include "game/entities/EPlayer.h"
#include "editor/EditorMain.h"
#include "editor/tools/InputRecorder.h"
#include "editor/EditorState.h"
#include "engine/io/display.h"
#include "editor/console/console.h"
#include "game/gameplay/GameState.h"
#include "game/input/PlayerInput.h"
#include "editor/EditorInput.h"
#include "engine/camera/camera.h"
#include "engine/render/ImRender.h"
#include "engine/render/renderer.h"
#include "engine/world/World.h"

void StartFrame();

void RavenousMainLoop()
{
	auto* ES = REditorState::Get();
	auto* World = RWorld::Get();
	auto* CamManager = RCameraManager::Get();

	while (!glfwWindowShouldClose(GlobalDisplayState::Get()->GetWindow()))
	{
		auto* Player = EPlayer::Get();

		// -------------
		//	INPUT PHASE
		// -------------
		// This needs to be first or dearImGUI will crash.
		auto InputFlags = StartInputPhase();

		auto* InputRecorder = RInputRecorder::Get();
		// Input recorder
		if (InputRecorder->bIsRecording) {
			InputRecorder->Record(InputFlags);
		}
		else if (InputRecorder->bIsPlaying) {
			InputFlags = InputRecorder->Play();
		}

		// -------------
		// START FRAME
		// -------------
		RavenousEngine::StartFrame();
		if (ES->CurrentMode == REditorState::ProgramMode::Editor) {
			Editor::StartDearImguiFrame();
		}

		// ---------------
		// INPUT HANDLING
		// ---------------
		auto* Camera = CamManager->GetCurrentCamera();

		if (REditorState::IsInConsoleMode()) {
			HandleConsoleInput(InputFlags, Player, World, Camera);
		}
		else
		{
			if (REditorState::IsInEditorMode())
			{
				Editor::HandleInputFlagsForEditorMode(InputFlags, World);
				
				if (!ImGui::GetIO().WantCaptureKeyboard) {
					InHandleMovementInput(InputFlags, Player, World);
					Editor::HandleInputFlagsForCommonInput(InputFlags, Player);
				}
			}
			else if (REditorState::IsInGameMode()) {
				InHandleMovementInput(InputFlags, Player, World);
				Editor::HandleInputFlagsForCommonInput(InputFlags, Player);
			}
		}
		ResetInputFlags(InputFlags);

		// -------------
		//	UPDATE PHASE
		// -------------
		{
			World->UpdateTraits();
			
			if (ES->CurrentMode == REditorState::ProgramMode::Game) {
				CamManager->UpdateGameCamera(GlobalDisplayState::ViewportWidth, GlobalDisplayState::ViewportHeight, Player->GetEyePosition());
			}
			else if (ES->CurrentMode == REditorState::ProgramMode::Editor) {
				CamManager->UpdateEditorCamera(GlobalDisplayState::ViewportWidth, GlobalDisplayState::ViewportHeight, Player->Position);
			}
			
			RGameState::Get()->UpdateTimers();
			Player->UpdateState();
			AnAnimatePlayer(Player);
			EntityAnimations.UpdateAnimations();
		}

		// -------------
		//	RENDER PHASE
		// -------------
		{
			auto& Frame = RavenousEngine::GetFrame();

			glClearColor(0.196f, 0.298f, 0.3607f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderDepthMap();
			RenderDepthCubemap();
			RenderScene(World, Camera);
			//render_depth_map_debug();
			switch (ES->CurrentMode)
			{
				case REditorState::ProgramMode::Console:
				{
					RenderConsole();
					break;
				}
				case REditorState::ProgramMode::Editor:
				{
					Editor::Update(Player, World, Camera);
					Editor::Render(Player, World, Camera);
					break;
				}
				case REditorState::ProgramMode::Game:
				{
					RenderGameGui(Player);
					break;
				}
			}
			RImDraw::Render(Camera);
			RImDraw::Update(Frame.Duration);
			Rvn::EditorMsgManager->Render();
		}

		// -------------
		// FINISH FRAME
		// -------------
		Rvn::EditorMsgManager->Update();
		World->DeleteEntitiesMarkedForDeletion();
		glfwSwapBuffers(GlobalDisplayState::Get()->GetWindow());
		if (ES->CurrentMode == REditorState::ProgramMode::Editor) {
			Editor::EndDearImguiFrame();
		}
	}

	glfwTerminate();
}
