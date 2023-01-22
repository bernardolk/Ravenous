/* ==========================================
                     RAVENOUS
   ==========================================
     By Bernardo L. Knackfuss - 2020 - 2023 
   ========================================== */


// DEPENDENCY INCLUDES
#include "engine/core/platform.h"
#include "engine/core/core.h"

#include <engine/core/ui.h>
#include "engine/io/input.h"
#include "engine/io/display.h"

#include <engine/logging.h>
#include <engine/rvn.h>
#include <engine/render/text/character.h>

#include "editor/tools/input_recorder.h"
#include "engine/world/scene_manager.h"

#include "engine/engine_state.h"
#include "engine/platform/gl.h"


/**
 *	Refactor plan:
 *	
 *	1. Create a engine/core.h and put all essentials there (including fwd declarations of common types)
 *	2. Start moving methods from outer scope to static types that implement a "static T* Get()" and refactor global instances to use that method
 *	3. Refactor ravenous.cpp: Delete unused code, make core loop very high level (refactor input/update/render loops into its own Engine methods)
 *	4. Implement cpp files for header-only code (excluding editor code) and move them into proper place in file structure
 *	5. Make inputs work in this build
 *
 *	Next steps plan:
 *
 *	1. Finish reflection system to work with multiple inheritance
 *	2. Implement the MemoryArena and EntityManager code from the loose files into here
 *	3. Refactor current entity types (doors, markings, etc) into actual entity types
 *	4. Make se/deserialization work for these types
 *	5. Profit $$$
 *	
 */


ProgramConfig GConfig;

// should be conditional in the future to support multiple platforms and
// we must abstract the function calls to a common layer which can interop
// between platform layers depending on the underlying OS.
#include <engine/platform/platform.h>


// SOURCE INCLUDES
#include <colors.h>
#include "engine/loop/input_phase.h"
#include <engine/collision/cl_types.h>
#include <engine/collision/primitives/ray.h>
#include <engine/collision/primitives/triangle.h>
#include <engine/collision/primitives/bounding_box.h>
#include <engine/vertex.h>
#include <engine/mesh.h>
#include <utils.h>
#include "engine/render/shader.h"
#include <engine/collision/collision_mesh.h>
#include "engine/entities/entity.h"
#include <engine/lights.h>
#include <scene.h>
#include <entity_state.h>
#include <player.h>
#include <engine/camera.h>
#include <engine/collision/raycast.h>
#include <engine/world/world.h>
#include <editor/tools/input_recorder.h>
#include <engine/entity_pool.h>

#include <engine/render/text/face.h>
#include <engine/render/text/text_renderer.h>
#include <engine/loaders.h>
#include <engine/entity_manager.h>
#include <geometry.h>

void toggle_program_modes(Player* player);
void erase_entity(Scene* scene, Entity* entity);

#include <engine/render/renderer.h>
#include <render.h>
#include <engine/render/im_render.h>
#include "engine/loop/input_phase.h"

#include <engine/collision/simplex.h>
#include <engine/collision/cl_gjk.h>
#include <engine/collision/cl_epa.h>
// #include <cl_log.h>
#include <engine/collision/cl_resolvers.h>
#include <engine/collision/cl_controller.h>
#include <game/collision/cl_edge_detection.h>
#include <gp_player_state.h>
#include <an_player.h>
#include <an_update.h>
#include <gp_timer.h>
#include <gp_game_state.h>
#include <gp_update.h>

#include <engine/serialization/sr_config.h>
#include <engine/serialization/sr_light.h>
#include <engine/serialization/sr_player.h>
#include <engine/serialization/sr_entity.h>
#include <engine/serialization/sr_world.h>
#include <console.h>
#include <in_handlers.h>

#include <editor/editor_main.h>

#define glCheckError() glCheckError_(__FILE__, __LINE__)

// OPENGL OBJECTS
unsigned int Texture, TextureSpecular;

// FUNCTION PROTOTYPES
void render_ray();
void update_scene_objects(World* world);
void initialize_shaders();

void start_frame();
void check_all_entities_have_shaders(World* world);
void check_all_entities_have_ids(World* world);
void check_all_geometry_has_gl_data();

int main()
{
	auto* ES = EngineState::Get();
	World world;

	auto* EM = EntityManager::Get();
	EM->SetWorld(&world);
	EM->SetEntityRegistry(&world.entities);
	EM->SetCheckpointsRegistry(&world.checkpoints);
	EM->SetInteractablesRegistry(&world.interactables);

	// initialize serializers

	//@TODO: This here is not working because EntityManager copy constructor was deleted. This is an issue
	//    with using references it seems? A pointer would never complain about this. I should dig into this.
	//    If I have to start writing extra code to use references then I can't justify using them.
	WorldSerializer::world = &world;
	WorldSerializer::manager = EM;
	PlayerSerializer::world = &world;
	LightSerializer::world = &world;
	EntitySerializer::manager = EM;
	ConfigSerializer::scene_info = GlobalSceneInfo::Get();

	// INITIAL GLFW AND GLAD SETUPS
	setup_GLFW(true);
	setup_gl();

	// create cameras

	auto* GSI = GlobalSceneInfo::Get();

	// load shaders, textures and geometry
	stbi_set_flip_vertically_on_load(true);
	load_textures_from_assets_folder();
	initialize_shaders();
	load_models();

	// Allocate buffers and logs
	Rvn::Init();
	std::cout << " BUFFER: " << Rvn::entity_buffer;
	// COLLISION_LOG           = CL_allocate_collision_log();
	initialize_console_buffers();

	EM->pool.Init();

	// Initialises immediate draw
	ImDraw::Init();

	// loads initial scene
	GConfig = ConfigSerializer::load_configs();
	WorldSerializer::load_from_file(GConfig.initial_scene);
	Player* player = GSI->player;
	world.player = player;
	player->checkpoint_pos = player->entity_ptr->position; // set player initial checkpoint position

	// set scene attrs from global config
	GSI->camera->acceleration = GConfig.camspeed;
	world.ambient_light = GConfig.ambient_light;
	world.ambient_intensity = GConfig.ambient_intensity;

	world.UpdateEntityWorldCells(player->entity_ptr); // sets player to the world
	CL_recompute_collision_buffer_entities(player);   // populates collision buffer and others

	Editor::initialize();

	// render features initialization
	create_depth_buffer();
	create_light_space_transform_matrices();

	// Pre-loop checks
	check_all_entities_have_shaders(&world);
	check_all_entities_have_ids(&world);
	check_all_geometry_has_gl_data();

	// load pre recorded input recordings
	InputRecorder.Load();

	// create hardcoded animations
	AN_create_hardcoded_animations();

	//@TODO: better for debugging
	player->entity_ptr->flags |= EntityFlags_RenderWireframe;

	// MAIN LOOP
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
			handle_console_input(input_flags, player, &world, GSI->camera);
		}
		else
		{
			if(EngineState::IsInEditorMode())
			{
				Editor::handle_input_flags(input_flags, player, &world, GSI->camera);
				if(!ImGui::GetIO().WantCaptureKeyboard)
				{
					IN_handle_movement_input(input_flags, player, &world);
					IN_handle_common_input(input_flags, player);
				}
			}
			else if(EngineState::IsInGameMode())
			{
				IN_handle_movement_input(input_flags, player, &world);
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
			GP_update_player_state(player, &world);
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
			render_depth_map(&world);
			render_depth_cubemap(&world);
			render_scene(&world, GSI->camera);
			//render_depth_map_debug();
			switch(ES->current_mode)
			{
				case EngineState::ProgramMode::Console:
					render_console();
					break;
				case EngineState::ProgramMode::Editor:
					Editor::update(player, &world, GSI->camera);
					Editor::render(player, &world, GSI->camera);
					break;
				case EngineState::ProgramMode::Game:
					render_game_gui(player);
					break;
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


//    ----------------------------------------------------------------


void start_frame()
{
	float current_frame_time = glfwGetTime();
	Rvn::frame.real_duration = current_frame_time - Rvn::frame.last_frame_time;
	Rvn::frame.duration = Rvn::frame.real_duration * Rvn::frame.time_step;
	Rvn::frame.last_frame_time = current_frame_time;

	// forces framerate for simulation to be small
	if(Rvn::frame.duration > 0.02)
	{
		Rvn::frame.duration = 0.02;
	}

	Rvn::frame.sub_second_counter += Rvn::frame.real_duration;
	Rvn::frame.fps_counter += 1;
	if(Rvn::frame.sub_second_counter > 1)
	{
		Rvn::frame.fps = Rvn::frame.fps_counter;
		Rvn::frame.fps_counter = 0;
		Rvn::frame.sub_second_counter -= 1;
	}
}

void check_all_entities_have_shaders(World* world)
{
	For(world->entities.size())
	{
		auto entity = world->entities[i];

		if(entity->shader == nullptr)
			Quit_fatal("shader not set for entity '" + entity->name + "'.");

		if(entity->mesh->gl_data.VAO == 0)
			Quit_fatal("GL DATA not set for entity '" + entity->name + "'.");
	}
}

void check_all_entities_have_ids(World* world)
{
	For(world->entities.size())
	{
		auto entity = world->entities[i];

		if(entity->name != PlayerName && entity->id == -1)
			Quit_fatal("There are entities without IDs. Check scene loading code for a flaw.");
	}
}

void check_all_geometry_has_gl_data()
{
	ForIt(GeometryCatalogue)
	{
		auto item = it->second;
		if(item->gl_data.VAO == 0 || item->gl_data.VBO == 0)
		{
			assert(false);
		}
	}
}

