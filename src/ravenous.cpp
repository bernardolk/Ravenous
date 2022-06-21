
/* ==========================================
                     RAVENOUS
   ==========================================
     By Bernardo L. Knackfuss - 2020 - 2022 
   ========================================== */


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

#include <dearIMGUI/imgui.h>
#include <dearIMGUI/imgui_impl_glfw.h>
#include <dearIMGUI/imgui_impl_opengl3.h>
#include <dearIMGUI/imgui_stdlib.h>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include <engine/core/rvn_types.h>
#include <engine/logging.h>
#include <engine/configs.h>
#include <engine/catalogues.h>


// @todo temp for missile action
bool Exploded = false;


const glm::mat4 mat4identity(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

// GLOBAL STRUCT VARIABLES OR TYPES 
enum ProgramModeEnum {
   GAME_MODE = 0,
   EDITOR_MODE = 1,
   CONSOLE_MODE = 2,
};

struct ProgramMode {
   ProgramModeEnum current = EDITOR_MODE;
   ProgramModeEnum last = EDITOR_MODE;
} PROGRAM_MODE;

struct GLData {
   GLuint VAO = 0;
   GLuint VBO = 0;
   GLuint EBO = 0;
};


GlobalDisplayInfo G_DISPLAY_INFO;

struct MouseCoordinates {
   double last_x = 0;
   double last_y = 0; 
   double click_x;
   double click_y;
   double x;
   double y;
};

struct GlobalInputInfo {
   bool forget_last_mouse_coords = true;
   MouseCoordinates mouse_coords;
   u64 key_state = 0;
   u8 mouse_state = 0;
   bool block_mouse_move = false;
} G_INPUT_INFO;

struct GlobalFrameInfo {
   float duration;
   float real_duration;
   float last_frame_time;
   int   fps;
   int   fps_counter;
   float sub_second_counter;
   float time_step = 1;
} G_FRAME_INFO;

struct ProgramConfig {
  std::string initial_scene;
   float camspeed;
   vec3 ambient_light;
   float ambient_intensity;
} G_CONFIG;

// should be conditional in the future to support multiple platforms and
// we must abstract the function calls to a common layer which can interop
// between platform layers depending on the underlying OS.
#include <rvn_platform.h>


// SOURCE INCLUDES
#include <rvn_macros.h>
#include <colors.h>
#include <in_flags.h>
#include <cl_types.h>
#include <texture.h>
#include <mesh.h>
#include <utils.h>
#include <character.h>
#include <engine/shader.h>
#include <entities.h>
#include <lights.h>
#include <scene.h>
#include <entity_state.h>
#include <player.h>
#include <engine/camera.h>
#include <engine/parser.h>
#include <cl_collider.h>
#include <world.h>
#include <input_recorder.h>
#include <globals.h>
#include <entity_pool.h>
#include <loaders.h>
#include <entity_manager.h>
#include <geometry.h>

// entity manager and entity pool
EntityManager Entity_Manager;

// camera handles
Camera* pCam;
Camera* edCam;

void toggle_program_modes(Player* player);
void erase_entity(Scene* scene, Entity* entity);
#include <editor/editor_im_macros.h>


#include <cl_tests.h>
#include <raycast.h>
#include <render.h>
#include <im_render.h>
#include <in_phase.h>
#include <gp_player_state.h>
#include <an_player.h>
#include <an_update.h>
#include <cl_buffers.h>
#include <cl_controller.h>
#include <gp_timer.h>
#include <gp_game_state.h>
#include <gp_update.h>

#include <sr_common.h>
#include <sr_save_scene.h>
#include <sr_save_configs.h>
#include <sr_load_scene.h>
#include <console.h>
#include <in_handlers.h>

#include <editor/editor_main.h>

#include <missile.h>
#include <compass.h>


#define glCheckError() glCheckError_(__FILE__, __LINE__) 

// OPENGL OBJECTS
unsigned int texture, texture_specular;

// FUNCTION PROTOTYPES
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void setup_GLFW(bool debug);
void render_ray();
void update_scene_objects();
void initialize_shaders();
GLenum glCheckError_(const char* file, int line);
void start_frame();
void check_all_entities_have_shaders();
void check_all_entities_have_ids();
void check_all_geometry_has_gl_data();
void setup_gl();
void simulate_gravity_trajectory();


int main()
{
   // INITIAL GLFW AND GLAD SETUPS
	setup_GLFW(true);
   setup_gl();

   // create cameras
	Camera* editor_camera = new Camera();
   Camera* first_person_camera = new Camera();
   G_SCENE_INFO.views[EDITOR_CAM] = editor_camera;
   G_SCENE_INFO.views[FPS_CAM] = first_person_camera;
   pCam = first_person_camera;
   edCam = editor_camera;

	// load shaders, textures and geometry
   stbi_set_flip_vertically_on_load(true);  
   load_textures_from_assets_folder();
   initialize_shaders();
   load_models();

   // Allocate buffers and logs
   G_BUFFERS.entity_buffer = allocate_entity_buffer();
   G_BUFFERS.rm_buffer     = allocate_render_message_buffer();
   COLLISION_LOG           = CL_allocate_collision_log();
   initialize_console_buffers();

   Entity_Manager.pool.init();

   // Initialises immediate draw
   IM_RENDER.init();

   // loads initial scene
   G_CONFIG = load_configs();
   load_scene_from_file(G_CONFIG.initial_scene, &World);
   Player* player = G_SCENE_INFO.player;
   player->checkpoint_pos = player->entity_ptr->position;   // set player initial checkpoint position

   // set scene attrs from global config
   G_SCENE_INFO.camera->Acceleration               = G_CONFIG.camspeed;
   G_SCENE_INFO.active_scene->ambient_light        = G_CONFIG.ambient_light;
   G_SCENE_INFO.active_scene->ambient_intensity    = G_CONFIG.ambient_intensity;

   Entity_Manager.set_default_entity_attributes(            // sets some loaded assets from scene as
      "aabb", "model", "grey"                               // defaults for entity construction
   );  
   World.update_entity_world_cells(player->entity_ptr);     // sets player to the world
   CL_recompute_collision_buffer_entities(player);          // populates collision buffer and others
   
   Editor::initialize();

   // render features initialization
   create_depth_buffer();
   create_light_space_transform_matrices();

   // Pre-loop checks
   check_all_entities_have_shaders();
   check_all_entities_have_ids();
   check_all_geometry_has_gl_data();

   // load pre recorded input recordings
   Input_Recorder.load();

   // create hardcoded animations
   AN_create_hardcoded_animations();

   //@TODO: better for debugging
   player->entity_ptr->flags |= EntityFlags_RenderWireframe;

   // Does a first update
   update_scene_objects();

   // -------------- MISSILE ------
   Entity* Missile_Entity  = G_SCENE_INFO.active_scene->find_entity("missile");
   Entity* Phone_Entity    = G_SCENE_INFO.active_scene->find_entity("phone");

	// MAIN LOOP
	while (!glfwWindowShouldClose(G_DISPLAY_INFO.window))
	{
      // -------------
		//	INPUT PHASE
      // -------------
      // This needs to be first or dearImGUI will crash.
      auto input_flags = input_phase();

      // Input recorder
      if(Input_Recorder.is_recording)
         Input_Recorder.record(input_flags);
      else if(Input_Recorder.is_playing)
         input_flags = Input_Recorder.play();

      // -------------
      // START FRAME
      // -------------
		start_frame();
      if(PROGRAM_MODE.current == EDITOR_MODE)
         Editor::start_dear_imgui_frame();

      // ---------------
      // INPUT HANDLING
      // ---------------
      switch(PROGRAM_MODE.current)
      {
         case CONSOLE_MODE:
            handle_console_input(input_flags, player, &World, G_SCENE_INFO.camera);
            break;
         case EDITOR_MODE:
            Editor::handle_input_flags(input_flags, player);
            if(!ImGui::GetIO().WantCaptureKeyboard)
            {
               IN_handle_movement_input(input_flags, player, EDITOR_MODE);
               IN_handle_common_input(input_flags, player);
            }
            break;
         case GAME_MODE:
            IN_handle_movement_input(input_flags, player, GAME_MODE);
            IN_handle_common_input(input_flags, player);
            break;
      }

      // if(pressed_once(input_flags, KEY_U))
      // {
      //    UPDATE_MISSILE = !UPDATE_MISSILE;
      // }

      if(pressed_once(input_flags, KEY_U))
      {
         UPDATE_MISSILE = true;
      }

      reset_input_flags(input_flags);

      // -------------
		//	UPDATE PHASE
      // -------------
      Frame_Ray_Collider_Count = 0;
      if(PROGRAM_MODE.current == GAME_MODE)
		   camera_update_game(G_SCENE_INFO.camera, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT, player->eye());
      else if(PROGRAM_MODE.current == EDITOR_MODE)
		   camera_update_editor(G_SCENE_INFO.camera, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT, player->entity_ptr->position);
      Game_State.update_timers();
      GP_update_player_state(player);
      AN_animate_player(player);
      Entity_Animations.update_animations();
     
      if(UPDATE_MISSILE && !Launch)
      {
         editor_print("Missile launch!");
         Launch = true;
      }

      if(UPDATE_MISSILE && !Exploded)
         update_missile(player, Missile_Entity);
      else if(Exploded)
      {
         // Entity_Manager.mark_for_deletion(Missile_Entity);
         Missile_Entity->flags |= EntityFlags_HiddenEntity;
      }

      get_compass_heading(Phone_Entity);

      //update_scene_objects();

      // UPDATE_MISSILE = false;

      // simulate_gravity_trajectory();      

      // -------------
		//	RENDER PHASE
      // -------------
		glClearColor(0.196, 0.298, 0.3607, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      render_depth_map();
      render_depth_cubemap();
		render_scene(G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
      //render_depth_map_debug();
      switch(PROGRAM_MODE.current)
      {
         case CONSOLE_MODE:
            render_console();
            break;
         case EDITOR_MODE:
            Editor::update();
            Editor::render(player, &World);
            break;
         case GAME_MODE:
            render_game_gui(player);
            break;
      }
      IM_RENDER.render();
      IM_RENDER.check_expired_entries();
      render_message_buffer_contents();

      // -------------
      // FINISH FRAME
      // -------------
      Entity_Manager.safe_delete_marked_entities();
      expire_render_messages_from_buffer();
		glfwSwapBuffers(G_DISPLAY_INFO.window);
      if(PROGRAM_MODE.current == EDITOR_MODE) 
         Editor::end_dear_imgui_frame();
	}

	glfwTerminate();
	return 0;
}

//    ----------------------------------------------------------------
void simulate_gravity_trajectory()
{
   // configs
   vec3 initial_pos   = vec3(2.0, 1.5, 6.5);
   vec3 v_direction  = -UNIT_X;
   float v_magnitude = 3;
   vec3 grav  = vec3(0, -9.0, 0);          // m/s^2
   int  iterations = 20;
   float d_frame = 0.02;

   // state
   vec3 vel     = v_direction * v_magnitude;
   vec3 pos     = initial_pos;


   for(int i = 0; i < iterations; i++)
   {
      vel += d_frame * grav;
      pos += vel * d_frame;
      IM_RENDER.add_point(IM_ITERHASH(i), pos, 2.0, false, COLOR_GREEN_1, 1);
   }
}

//    ----------------------------------------------------------------


void start_frame()
{
   float current_frame_time = glfwGetTime();
   G_FRAME_INFO.real_duration = current_frame_time - G_FRAME_INFO.last_frame_time;
   G_FRAME_INFO.duration = G_FRAME_INFO.real_duration * G_FRAME_INFO.time_step;
   G_FRAME_INFO.last_frame_time = current_frame_time;

   // forces framerate for simulation to be small
   if(G_FRAME_INFO.duration > 0.02)
   {
      G_FRAME_INFO.duration = 0.02;
   } 

   G_FRAME_INFO.sub_second_counter += G_FRAME_INFO.real_duration;
   G_FRAME_INFO.fps_counter        += 1;
   if(G_FRAME_INFO.sub_second_counter > 1)
   {
      G_FRAME_INFO.fps                 = G_FRAME_INFO.fps_counter;
      G_FRAME_INFO.fps_counter         = 0;
      G_FRAME_INFO.sub_second_counter -= 1;
   }
}

void check_all_entities_have_shaders()
{
   For(G_SCENE_INFO.active_scene->entities.size())
   {
	   auto entity = G_SCENE_INFO.active_scene->entities[i];

      if(entity->shader == nullptr)
         Quit_fatal("shader not set for entity '" + entity->name + "'.");

      if(entity->mesh->gl_data.VAO == 0)
         Quit_fatal("GL DATA not set for entity '" + entity->name + "'.");
   }
}

void check_all_entities_have_ids()
{
	For(G_SCENE_INFO.active_scene->entities.size())
   {
	   auto entity = G_SCENE_INFO.active_scene->entities[i];

      if(entity->name != PLAYER_NAME && entity->id == -1)
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


inline void update_scene_objects() 
{
	Entity** entity_iterator = &G_SCENE_INFO.active_scene->entities[0];
	size_t list_size = G_SCENE_INFO.active_scene->entities.size();
	for (int i = 0; i < list_size; i++) 
   {
      Entity* &entity = *entity_iterator;
		// Updates model matrix;	
		entity->update();
      entity_iterator++;

      // auto[min,max] = entity->bounding_box.bounds();
      // IM_RENDER.add_point(IMHASH, min, 3.0, true, vec3(0.964, 0.576, 0.215));
      // IM_RENDER.add_point(IMHASH, max, 3.0, true, vec3(0.964, 0.576, 0.215));
	}
}


void setup_GLFW(bool debug) {
	// Setup the window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_SAMPLES, 4);

	// Creates the window
	G_DISPLAY_INFO.window = glfwCreateWindow(GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT, "Ravenous", NULL, NULL);
	if (G_DISPLAY_INFO.window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(G_DISPLAY_INFO.window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Setups openGL viewport
	glViewport(0, 0, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT);
	glfwSetFramebufferSizeCallback(G_DISPLAY_INFO.window, framebuffer_size_callback);
	glfwSetCursorPosCallback(G_DISPLAY_INFO.window, on_mouse_move);
	glfwSetScrollCallback(G_DISPLAY_INFO.window, on_mouse_scroll);
	glfwSetMouseButtonCallback(G_DISPLAY_INFO.window, on_mouse_btn);

	if (debug) {
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	}

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
      //case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
      //case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}

void toggle_program_modes(Player* player)
{
   G_INPUT_INFO.forget_last_mouse_coords = true;
   
   if(PROGRAM_MODE.current == EDITOR_MODE)
   {
      PROGRAM_MODE.last    = PROGRAM_MODE.current;
      PROGRAM_MODE.current = GAME_MODE;
      G_SCENE_INFO.camera  = G_SCENE_INFO.views[1];
      player->entity_ptr->flags |= EntityFlags_InvisibleEntity;
      glfwSetInputMode(G_DISPLAY_INFO.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      Editor::end_dear_imgui_frame();

      G_BUFFERS.rm_buffer->add("Game Mode", 2000);
   }
   else if(PROGRAM_MODE.current == GAME_MODE)
   {
      PROGRAM_MODE.last    = PROGRAM_MODE.current;
      PROGRAM_MODE.current = EDITOR_MODE;
      G_SCENE_INFO.camera  = G_SCENE_INFO.views[0];
      player->entity_ptr->flags &= ~EntityFlags_InvisibleEntity;
      glfwSetInputMode(G_DISPLAY_INFO.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      Editor::start_dear_imgui_frame();

      G_BUFFERS.rm_buffer->add("Editor Mode", 2000);
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