#include "engine/MainLoop.h"

#include <glfw3.h>
#include <imgui.h>

#include "game/animation/AnPlayer.h"
#include "game/animation/AnUpdate.h"
#include "game/entities/player.h"
#include "editor/editor.h"
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
#include "engine/world/world.h"

void StartFrame();

void RavenousMainLoop()
{
	auto* ES = EditorState::Get();
	auto player = Player::Get();
	auto world = World::Get();
	auto* cam_manager = CameraManager::Get();
	
	while (!glfwWindowShouldClose(GlobalDisplayState::Get()->GetWindow()))
	{
		// -------------
		//	INPUT PHASE
		// -------------
		// This needs to be first or dearImGUI will crash.
		auto input_flags = InputPhase();

		auto* input_recorder = InputRecorder::Get();
		// Input recorder
		if (input_recorder->is_recording)
			input_recorder->Record(input_flags);
		else if (input_recorder->is_playing)
			input_flags = input_recorder->Play();

		// -------------
		// START FRAME
		// -------------
		StartFrame();
		if (ES->current_mode == EditorState::ProgramMode::Editor)
			Editor::StartDearImguiFrame();

		// ---------------
		// INPUT HANDLING
		// ---------------
		auto* camera = cam_manager->GetCurrentCamera();

		if (EditorState::IsInConsoleMode())
		{
			HandleConsoleInput(input_flags, player, world, camera);
		}
		else
		{
			if (EditorState::IsInEditorMode())
			{
				Editor::HandleInputFlagsForEditorMode(input_flags, world);
				if (!ImGui::GetIO().WantCaptureKeyboard)
				{
					IN_HandleMovementInput(input_flags, player, world);
					Editor::HandleInputFlagsForCommonInput(input_flags, player);
				}
			}
			else if (EditorState::IsInGameMode())
			{
				IN_HandleMovementInput(input_flags, player, world);
				Editor::HandleInputFlagsForCommonInput(input_flags, player);
			}
		}
		ResetInputFlags(input_flags);

		// -------------
		//	UPDATE PHASE
		// -------------
		{
			if (ES->current_mode == EditorState::ProgramMode::Game)
			{
				cam_manager->UpdateGameCamera(GlobalDisplayState::viewport_width, GlobalDisplayState::viewport_height, player->GetEyePosition());
			}
			else if (ES->current_mode == EditorState::ProgramMode::Editor)
			{
				cam_manager->UpdateEditorCamera(GlobalDisplayState::viewport_width, GlobalDisplayState::viewport_height, player->position);
			}
			GameState::Get()->UpdateTimers();
			player->UpdateState();
			AN_AnimatePlayer(player);
			EntityAnimations.UpdateAnimations();
		}

		// -------------
		//	RENDER PHASE
		// -------------
		{
			glClearColor(0.196, 0.298, 0.3607, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderDepthMap();
			RenderDepthCubemap();
			RenderScene(world, camera);
			//render_depth_map_debug();
			switch (ES->current_mode)
			{
				case EditorState::ProgramMode::Console:
				{
					RenderConsole();
					break;
				}
				case EditorState::ProgramMode::Editor:
				{
					Editor::Update(player, world, camera);
					Editor::Render(player, world, camera);
					break;
				}
				case EditorState::ProgramMode::Game:
				{
					RenderGameGui(player);
					break;
				}
			}
			ImDraw::Render(camera);
			ImDraw::Update(Rvn::frame.duration);
			Rvn::rm_buffer->Render();
		}

		// -------------
		// FINISH FRAME
		// -------------
		Rvn::rm_buffer->Cleanup();
		glfwSwapBuffers(GlobalDisplayState::Get()->GetWindow());
		if (ES->current_mode == EditorState::ProgramMode::Editor)
			Editor::EndDearImguiFrame();
	}

	glfwTerminate();
}
