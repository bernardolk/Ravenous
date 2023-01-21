/* ==========================================
                     RAVENOUS
   ==========================================
     By Bernardo L. Knackfuss - 2020 - 2022 
   ========================================== */
#include <iostream>


// DEPENDENCY INCLUDES
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <algorithm>
#include <stdint.h>
#include <chrono>

#include <dearIMGUI/imgui.h>
#include <dearIMGUI/imgui_impl_glfw.h>
#include <dearIMGUI/imgui_impl_opengl3.h>
#include <dearIMGUI/imgui_stdlib.h>

#include <stb_image/stb_image.h>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include <engine/core/rvn_types.h>
#include <engine/logging.h>
#include <engine/rvn.h>
#include <engine/render/text/character.h>

// GLOBAL STRUCT VARIABLES OR TYPES 
enum ProgramModeEnum
{
	GAME_MODE    = 0,
	EDITOR_MODE  = 1,
	CONSOLE_MODE = 2,
};

// ReSharper disable once CppInconsistentNaming
struct T_ProgramMode
{
	ProgramModeEnum current = EDITOR_MODE;
	ProgramModeEnum last = EDITOR_MODE;
} ProgramMode;


GlobalDisplayConfig GDisplayInfo;

struct MouseCoordinates
{
	double last_x = 0;
	double last_y = 0;
	double click_x;
	double click_y;
	double x;
	double y;
};

struct GlobalInputInfo
{
	bool forget_last_mouse_coords = true;
	MouseCoordinates mouse_coords;
	u64 key_state = 0;
	u8 mouse_state = 0;
	bool block_mouse_move = false;
} GInputInfo;

ProgramConfig GConfig;

// should be conditional in the future to support multiple platforms and
// we must abstract the function calls to a common layer which can interop
// between platform layers depending on the underlying OS.
#include <rvn_platform.h>


// SOURCE INCLUDES
#include <rvn_macros.h>
#include <colors.h>
#include <in_flags.h>
#include <engine/collision/cl_types.h>
#include <engine/collision/primitives/ray.h>
#include <engine/collision/primitives/triangle.h>
#include <engine/collision/primitives/bounding_box.h>
#include <engine/vertex.h>
#include <engine/mesh.h>
#include <utils.h>
#include <engine/shader.h>
#include <engine/collision/collision_mesh.h>
#include <engine/entity.h>
#include <engine/lights.h>
#include <scene.h>
#include <entity_state.h>
#include <player.h>
#include <engine/camera.h>
#include <engine/collision/raycast.h>
#include <engine/world/world.h>
#include <input_recorder.h>
#include <engine/entity_pool.h>

#include <editor/editor_im_macros.h>
#include <engine/render/text/face.h>
#include <engine/render/text/text_renderer.h>
#include <engine/loaders.h>
#include <engine/entity_manager.h>
#include <geometry.h>

// entity manager and entity pool
T_EntityManager EntityManager;

// camera handles
Camera* PCam;
Camera* EdCam;

void toggle_program_modes(Player* player);
void erase_entity(Scene* scene, Entity* entity);

#include <engine/render/renderer.h>
#include <render.h>
#include <engine/render/im_render.h>
#include <in_phase.h>

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

// globals
GlobalSceneInfo GSceneInfo;

#define glCheckError() glCheckError_(__FILE__, __LINE__)

// OPENGL OBJECTS
unsigned int texture, texture_specular;

// FUNCTION PROTOTYPES
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void setup_GLFW(bool debug);
void render_ray();
void update_scene_objects(World* world);
void initialize_shaders();
GLenum glCheckError_(const char* file, int line);
void start_frame();
void check_all_entities_have_shaders(World* world);
void check_all_entities_have_ids(World* world);
void check_all_geometry_has_gl_data();
void setup_gl();
void simulate_gravity_trajectory();

static void get_time_update(int elapsed)
{
	static std::vector<int> times;
	const int N = 100;

	times.push_back(elapsed);


	if(times.size() == N)
	{
		int sum = 0;
		for(int i = 0; i < times.size(); i++)
			sum += times[i];

		float average = sum * 1.0 / N;

		std::cout << "Average Update Time: " << average << "\n";

		times.clear();
	}
}

static void get_time_render(int elapsed)
{
	static std::vector<int> times;
	const int N = 100;

	times.push_back(elapsed);


	if(times.size() == N)
	{
		int sum = 0;
		for(int i = 0; i < times.size(); i++)
			sum += times[i];

		float average = sum * 1.0 / N;

		std::cout << "Average Render Time: " << average << "\n";

		times.clear();
	}
}

int main()
{
	World world;

	EntityManager.set_world(&world);
	EntityManager.set_entity_registry(&world.entities);
	EntityManager.set_checkpoints_registry(&world.checkpoints);
	EntityManager.set_interactables_registry(&world.interactables);

	// initialize serializers

	//@TODO: This here is not working because EntityManager copy constructor was deleted. This is an issue
	//    with using references it seems? A pointer would never complain about this. I should dig into this.
	//    If I have to start writing extra code to use references then I can't justify using them.
	WorldSerializer::world = &world;
	WorldSerializer::manager = &EntityManager;
	PlayerSerializer::world = &world;
	LightSerializer::world = &world;
	EntitySerializer::manager = &EntityManager;
	ConfigSerializer::scene_info = &GSceneInfo;

	// INITIAL GLFW AND GLAD SETUPS
	setup_GLFW(true);
	setup_gl();

	// create cameras
	const auto editor_camera = new Camera();
	const auto first_person_camera = new Camera();
	GSceneInfo.views[EDITOR_CAM] = editor_camera;
	GSceneInfo.views[FPS_CAM] = first_person_camera;
	PCam = first_person_camera;
	EdCam = editor_camera;

	// load shaders, textures and geometry
	stbi_set_flip_vertically_on_load(true);
	load_textures_from_assets_folder();
	initialize_shaders();
	load_models();

	// Allocate buffers and logs
	RVN::init();
	std::cout << " BUFFER: " << RVN::entity_buffer;
	// COLLISION_LOG           = CL_allocate_collision_log();
	initialize_console_buffers();

	EntityManager.pool.init();

	// Initialises immediate draw
	ImDraw::init();

	GSceneInfo.camera = GSceneInfo.views[0]; // sets to editor camera


	// loads initial scene
	GConfig = ConfigSerializer::load_configs();
	WorldSerializer::load_from_file(GConfig.initial_scene);
	Player* player = GSceneInfo.player;
	world.player = player;
	player->checkpoint_pos = player->entity_ptr->position; // set player initial checkpoint position

	// set scene attrs from global config
	GSceneInfo.camera->Acceleration = GConfig.camspeed;
	world.ambient_light = GConfig.ambient_light;
	world.ambient_intensity = GConfig.ambient_intensity;

	world.UpdateEntityWorldCells(player->entity_ptr); // sets player to the world
	CL_recompute_collision_buffer_entities(player);      // populates collision buffer and others

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
	while(!glfwWindowShouldClose(GDisplayInfo.window))
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
		if(ProgramMode.current == EDITOR_MODE)
			Editor::start_dear_imgui_frame();

		// ---------------
		// INPUT HANDLING
		// ---------------
		switch(ProgramMode.current)
		{
		case CONSOLE_MODE:
			handle_console_input(input_flags, player, &world, GSceneInfo.camera);
			break;
		case EDITOR_MODE:
			Editor::handle_input_flags(input_flags, player, &world, GSceneInfo.camera);
			if(!ImGui::GetIO().WantCaptureKeyboard)
			{
				IN_handle_movement_input(input_flags, player, EDITOR_MODE, &world);
				IN_handle_common_input(input_flags, player);
			}
			break;
		case GAME_MODE:
			IN_handle_movement_input(input_flags, player, GAME_MODE, &world);
			IN_handle_common_input(input_flags, player);
			break;
		}

		reset_input_flags(input_flags);

		// -------------
		//	UPDATE PHASE
		// -------------
		{
			auto start = std::chrono::high_resolution_clock::now();
			if(ProgramMode.current == GAME_MODE)
				camera_update_game(GSceneInfo.camera, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT, player->Eye());
			else if(ProgramMode.current == EDITOR_MODE)
				camera_update_editor(GSceneInfo.camera, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT, player->entity_ptr->position);
			GameState.UpdateTimers();
			GP_update_player_state(player, &world);
			AN_animate_player(player);
			EntityAnimations.UpdateAnimations();
			auto finish = std::chrono::high_resolution_clock::now();
			int elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
			get_time_update(elapsed);
		}


		//update_scene_objects();

		// simulate_gravity_trajectory();      

		// -------------
		//	RENDER PHASE
		// -------------
		{
			auto start = std::chrono::high_resolution_clock::now();
			glClearColor(0.196, 0.298, 0.3607, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			render_depth_map(&world);
			render_depth_cubemap(&world);
			render_scene(&world, GSceneInfo.camera);
			//render_depth_map_debug();
			switch(ProgramMode.current)
			{
			case CONSOLE_MODE:
				render_console();
				break;
			case EDITOR_MODE:
				Editor::update(player, &world, GSceneInfo.camera);
				Editor::render(player, &world, GSceneInfo.camera);
				break;
			case GAME_MODE:
				render_game_gui(player);
				break;
			}
			ImDraw::render(GSceneInfo.camera);
			ImDraw::update(RVN::frame.duration);
			RVN::rm_buffer->render();
			auto finish = std::chrono::high_resolution_clock::now();
			int elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
			get_time_render(elapsed);
		}

		// -------------
		// FINISH FRAME
		// -------------
		EntityManager.safe_delete_marked_entities();
		RVN::rm_buffer->cleanup();
		glfwSwapBuffers(GDisplayInfo.window);
		if(ProgramMode.current == EDITOR_MODE)
			Editor::end_dear_imgui_frame();
	}


	glfwTerminate();
}

//    ----------------------------------------------------------------
void simulate_gravity_trajectory()
{
	// configs
	auto initial_pos = vec3(2.0, 1.5, 6.5);
	vec3 v_direction = -UNIT_X;
	float v_magnitude = 3;
	auto grav = vec3(0, -9.0, 0); // m/s^2
	int iterations = 20;

	// state
	vec3 vel = v_direction * v_magnitude;
	vec3 pos = initial_pos;


	for(int i = 0; i < iterations; i++)
	{
		float d_frame = 0.02;
		vel += d_frame * grav;
		pos += vel * d_frame;
		ImDraw::add_point(IM_ITERHASH(i), pos, 2.0, false, COLOR_GREEN_1, 1);
	}
}

//    ----------------------------------------------------------------


void start_frame()
{
	float current_frame_time = glfwGetTime();
	RVN::frame.real_duration = current_frame_time - RVN::frame.last_frame_time;
	RVN::frame.duration = RVN::frame.real_duration * RVN::frame.time_step;
	RVN::frame.last_frame_time = current_frame_time;

	// forces framerate for simulation to be small
	if(RVN::frame.duration > 0.02)
	{
		RVN::frame.duration = 0.02;
	}

	RVN::frame.sub_second_counter += RVN::frame.real_duration;
	RVN::frame.fps_counter += 1;
	if(RVN::frame.sub_second_counter > 1)
	{
		RVN::frame.fps = RVN::frame.fps_counter;
		RVN::frame.fps_counter = 0;
		RVN::frame.sub_second_counter -= 1;
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
	ForIt(Geometry_Catalogue)
	{
		auto item = it->second;
		if(item->gl_data.VAO == 0 || item->gl_data.VBO == 0)
		{
			assert(false);
		}
	}
}


void setup_GLFW(bool debug)
{
	// Setup the window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Creates the window
	GDisplayInfo.window = glfwCreateWindow(GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT, "Ravenous", nullptr, nullptr);
	if(GDisplayInfo.window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(GDisplayInfo.window);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Setups openGL viewport
	glViewport(0, 0, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT);
	glfwSetFramebufferSizeCallback(GDisplayInfo.window, framebuffer_size_callback);
	glfwSetCursorPosCallback(GDisplayInfo.window, on_mouse_move);
	glfwSetScrollCallback(GDisplayInfo.window, on_mouse_scroll);
	glfwSetMouseButtonCallback(GDisplayInfo.window, on_mouse_btn);

	if(debug)
	{
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	}

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLenum glCheckError_(const char* file, int line)
{
	GLenum error_code;
	while((error_code = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch(error_code)
		{
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		//case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		//case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		default: break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return error_code;
}

void toggle_program_modes(Player* player)
{
	GInputInfo.forget_last_mouse_coords = true;

	if(ProgramMode.current == EDITOR_MODE)
	{
		ProgramMode.last = ProgramMode.current;
		ProgramMode.current = GAME_MODE;
		GSceneInfo.camera = GSceneInfo.views[1];
		player->entity_ptr->flags |= EntityFlags_InvisibleEntity;
		glfwSetInputMode(GDisplayInfo.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		Editor::end_dear_imgui_frame();

		RVN::rm_buffer->add("Game Mode", 2000);
	}
	else if(ProgramMode.current == GAME_MODE)
	{
		ProgramMode.last = ProgramMode.current;
		ProgramMode.current = EDITOR_MODE;
		GSceneInfo.camera = GSceneInfo.views[0];
		player->entity_ptr->flags &= ~EntityFlags_InvisibleEntity;
		glfwSetInputMode(GDisplayInfo.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Editor::start_dear_imgui_frame();

		RVN::rm_buffer->add("Editor Mode", 2000);
	}
}

void setup_gl()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Sets opengl to require just 1 byte per pixel in textures
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_MULTISAMPLE);
}
