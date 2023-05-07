#include "engine/main_loop.h"

#include <glfw3.h>
#include <imgui.h>

#include "game/animation/an_player.h"
#include "game/animation/an_update.h"
#include "game/entities/player.h"
#include "editor/editor.h"
#include "editor/tools/input_recorder.h"
#include "editor/editor_state.h"
#include "engine/io/display.h"
#include "editor/console/console.h"
#include "game/gameplay/gp_game_state.h"
#include "game/input/player_input.h"
#include "editor/editor_input.h"
#include "engine/camera/camera.h"
#include "engine/render/im_render.h"
#include "engine/render/renderer.h"
#include "engine/world/world.h"

void StartFrame();

void RavenousMainLoop()
{
	auto* ES = EditorState::Get();
	auto player = Player::Get();
	auto world = T_World::Get();
	auto* cam_manager = CameraManager::Get();
	
	while (!glfwWindowShouldClose(GlobalDisplayConfig::GetWindow()))
	{
		// -------------
		//	INPUT PHASE
		// -------------
		// This needs to be first or dearImGUI will crash.
		auto input_flags = InputPhase();

		// Input recorder
		if (InputRecorder.is_recording)
			InputRecorder.Record(input_flags);
		else if (InputRecorder.is_playing)
			input_flags = InputRecorder.Play();

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
				cam_manager->UpdateGameCamera(GlobalDisplayConfig::viewport_width, GlobalDisplayConfig::viewport_height, player->GetEyePosition());
			}
			else if (ES->current_mode == EditorState::ProgramMode::Editor)
			{
				cam_manager->UpdateEditorCamera(GlobalDisplayConfig::viewport_width, GlobalDisplayConfig::viewport_height, player->position);
			}
			GameState.UpdateTimers();
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
		glfwSwapBuffers(GlobalDisplayConfig::GetWindow());
		if (ES->current_mode == EditorState::ProgramMode::Editor)
			Editor::EndDearImguiFrame();
	}

	glfwTerminate();
}
