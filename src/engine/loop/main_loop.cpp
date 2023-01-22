
#include "engine/loop/main_loop.h"

#include <glfw3.h>
#include <imgui.h>

#include "an_player.h"
#include "an_update.h"
#include "player.h"
#include "editor/editor.h"
#include "editor/tools/input_recorder.h"
#include "engine/engine_state.h"
#include "engine/io/display.h"
#include "engine/world/world.h"
#include "console.h"
#include "gp_game_state.h"
#include "gp_update.h"
#include "in_handlers.h"
#include "editor/editor_input.h"
#include "engine/camera.h"
#include "engine/render/im_render.h"
#include "engine/render/renderer.h"
#include "engine/world/scene_manager.h"

void start_frame();
void RavenousMainLoop()
{
	auto* ES = EngineState::Get();
	auto player = Player::Get();
	auto world = World::Get();
	auto* GSI = GlobalSceneInfo::Get();
	auto* EM = EntityManager::Get();
	
	while(!glfwWindowShouldClose(GlobalDisplayConfig::GetWindow()))
	{
		// -------------
		//	INPUT PHASE
		// -------------
		// This needs to be first or dearImGUI will crash.
		auto input_flags = input_phase();

		// Input recorder
		if(InputRecorder.is_recording)
			InputRecorder.Record(input_flags);
		else if(InputRecorder.is_playing)
			input_flags = InputRecorder.Play();

		// -------------
		// START FRAME
		// -------------
		start_frame();
		if(ES->current_mode == EngineState::ProgramMode::Editor)
			Editor::start_dear_imgui_frame();

		// ---------------
		// INPUT HANDLING
		// ---------------
		if(EngineState::IsInConsoleMode())
		{
			handle_console_input(input_flags, player, world, GSI->camera);
		}
		else
		{
			if(EngineState::IsInEditorMode())
			{
				Editor::handle_input_flags(input_flags, world, GSI->camera);
				if(!ImGui::GetIO().WantCaptureKeyboard)
				{
					IN_handle_movement_input(input_flags, player, world);
					IN_handle_common_input(input_flags, player);
				}
			}
			else if(EngineState::IsInGameMode())
			{
				IN_handle_movement_input(input_flags, player, world);
			}

			IN_handle_common_input(input_flags, player);
		}
		reset_input_flags(input_flags);

		// -------------
		//	UPDATE PHASE
		// -------------
		{
			if(ES->current_mode == EngineState::ProgramMode::Game)
				camera_update_game(GSI->camera, GlobalDisplayConfig::viewport_width, GlobalDisplayConfig::viewport_height, player->Eye());
			else if(ES->current_mode == EngineState::ProgramMode::Editor)
				camera_update_editor(GSI->camera, GlobalDisplayConfig::viewport_width, GlobalDisplayConfig::viewport_height, player->entity_ptr->position);
			GameState.UpdateTimers();
			GP_update_player_state(player, world);
			AN_animate_player(player);
			EntityAnimations.UpdateAnimations();
		}


		//update_scene_objects();

		// simulate_gravity_trajectory();      

		// -------------
		//	RENDER PHASE
		// -------------
		{
			glClearColor(0.196, 0.298, 0.3607, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			render_depth_map(world);
			render_depth_cubemap(world);
			render_scene(world, GSI->camera);
			//render_depth_map_debug();
			switch(ES->current_mode)
			{
				case EngineState::ProgramMode::Console:
				{
					render_console();
					break;
				}
				case EngineState::ProgramMode::Editor:
				{
					Editor::update(player, world, GSI->camera);
					Editor::render(player, world, GSI->camera);
					break;
				}
				case EngineState::ProgramMode::Game:
				{
					render_game_gui(player);
					break;
				}
			}
			ImDraw::Render(GSI->camera);
			ImDraw::Update(Rvn::frame.duration);
			Rvn::rm_buffer->Render();
		}

		// -------------
		// FINISH FRAME
		// -------------
		EM->SafeDeleteMarkedEntities();
		Rvn::rm_buffer->Cleanup();
		glfwSwapBuffers(GlobalDisplayConfig::GetWindow());
		if(ES->current_mode == EngineState::ProgramMode::Editor)
			Editor::end_dear_imgui_frame();
	}

	glfwTerminate();
}
