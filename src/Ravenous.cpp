
/* ==========================================
                     RAVENOUS
   ==========================================
     By Bernardo L. Knackfuss - 2020 - 2021 
   ========================================== */


// DEPENDENCY INCLUDES
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/vector_float2.hpp> // vec2
#include <glm/ext/vector_float3.hpp> // vec3
#include <glm/ext/matrix_float4x4.hpp> // mat4x4
#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
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


#include <rvn_types.h>


const string PROJECT_PATH = "c:/repositories/ravenous";
const string TEXTURES_PATH = PROJECT_PATH + "/assets/textures/";
const string MODELS_PATH = PROJECT_PATH + "/assets/models/";
const string FONTS_PATH = PROJECT_PATH + "/assets/fonts/";
const string SHADERS_FOLDER_PATH = PROJECT_PATH + "/shaders/";
const string CAMERA_FILE_PATH = PROJECT_PATH + "/camera.txt";
const string SCENES_FOLDER_PATH = PROJECT_PATH + "/scenes/";
const string SHADERS_FILE_EXTENSION = ".shd";
const string CONFIG_FILE_PATH = PROJECT_PATH + "/config.txt";
const string SCENE_TEMPLATE_NAME = "scene_template";
const string INPUT_RECORDINGS_FOLDER_PATH = PROJECT_PATH + "/recordings/";

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

struct GlobalDisplayInfo {
   GLFWwindow* window;
   const float VIEWPORT_WIDTH = 1980;
   const float VIEWPORT_HEIGHT = 1080;
} G_DISPLAY_INFO;

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
   string initial_scene;
   float camspeed;
   vec3 ambient_light;
   float ambient_intensity;
} G_CONFIG;

// should be conditional in the future to support multiple platforms and
// we must abstract the function calls to a common layer which can interop
// between platform layers depending on the underlying OS.
#include <rvn_platform.h>


// SOURCE INCLUDES
#include <colors.h>
#include <in_flags.h>
#include <cl_types.h>
#include <mesh.h>
#include <utils.h>
#include <character.h>
#include <shader.h>
#include <entities.h>
#include <entity_state.h>
#include <player.h>
#include <camera.h>
#include <parser.h>
#include <cl_collider.h>
#include <world.h>
#include <input_recorder.h>
#include <globals.h>
#include <entity_pool.h>
#include <loaders.h>
#include <entity_manager.h>

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
#include <cl_buffers.h>
#include <cl_controller.h>
#include <gp_update.h>
#include <scene.h>
#include <console.h>
#include <in_handlers.h>
#include <editor/editor_main.h>


#define glCheckError() glCheckError_(__FILE__, __LINE__) 

// OPENGL OBJECTS
unsigned int texture, texture_specular;

using namespace glm;

// FUNCTION PROTOTYPES
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void setup_GLFW(bool debug);
void render_ray();
void update_scene_objects();
void initialize_shaders();
void create_boilerplate_geometry();
GLenum glCheckError_(const char* file, int line);
void start_frame();
void check_all_entities_have_shaders();
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
   load_textures_from_assets_folder();
   initialize_shaders();
   create_boilerplate_geometry();

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

   // load pre recorded input recordings
   Input_Recorder.load();

   //@TODO: for debugging
   player->entity_ptr->wireframe = true;

   // Does a first update
   update_scene_objects();

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
      reset_input_flags(input_flags);

      // -------------
		//	UPDATE PHASE
      // -------------
      Frame_Ray_Collider_Count = 0;
		camera_update(G_SCENE_INFO.camera, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT, player);
      // @todo - check player events
      GP_update_player_state(player);
      AN_animate_player(player);
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
   Entity **entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;

      if(entity->shader == nullptr)
      {
         cout << "FATAL: shader not set for entity '" << entity->name << "'.\n";
         assert(false); 
      }
      if(entity->mesh->gl_data.VAO == 0)
      {
         cout << "FATAL: GL DATA not set for entity '" << entity->name << "'.\n";
         assert(false); 
      }
   }
}

void create_boilerplate_geometry()
{
   //TEXT
   GLData text_gl_data;
	glGenVertexArrays(1, &text_gl_data.VAO);
	glGenBuffers(1, &text_gl_data.VBO);
	glBindVertexArray(text_gl_data.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, text_gl_data.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

   Mesh* text_mesh = new Mesh();
   text_mesh->name = "text";
   text_mesh->gl_data = text_gl_data;
   Geometry_Catalogue.insert({text_mesh->name, text_mesh});

   // AABB
   vector<Vertex> aabb_vertex_vec = {
      // bottom
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.5f, 0.5f)},   //0
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 0.5f)},   //1
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 1.0f)},   //2
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.5f, 1.0f)},   //3
      // top   
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(0.0f, 1.0f, 0.0f),    vec2(0.5f, 0.5f)},   //4
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(0.0f, 1.0f, 0.0f),    vec2(1.0f, 0.5f)},   //5
      Vertex{vec3(1.0f, 1.0f, 1.0f),   vec3(0.0f, 1.0f, 0.0f),    vec2(1.0f, 1.0f)},   //6
      Vertex{vec3(1.0f, 1.0f, 0.0f),   vec3(0.0f, 1.0f, 0.0f),    vec2(0.5f, 1.0f)},   //7
      // front       
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.0f, 0.0f)},   //8
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.5f, 0.0f)},   //9
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.0f, 0.5f)},   //10
      Vertex{vec3(1.0f, 1.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.5f, 0.5f)},   //11
      // back
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),    vec2(0.0f, 0.0f)},   //12
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),    vec2(0.0f, 0.5f)},   //13
      Vertex{vec3(1.0f, 1.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),    vec2(0.5f, 0.5f)},   //14
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),    vec2(0.5f, 0.0f)},   //15
      // left
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.0f, 0.0f)},   //16
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.5f, 0.0f)},   //17
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.0f, 0.5f)},   //18
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.5f, 0.5f)},   //19
      // right
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(1.0f, 0.0f, 0.0f),    vec2(0.0f, 0.0f)},   //20
      Vertex{vec3(1.0f, 1.0f, 0.0f),   vec3(1.0f, 0.0f, 0.0f),    vec2(0.0f, 0.5f)},   //21
      Vertex{vec3(1.0f, 1.0f, 1.0f),   vec3(1.0f, 0.0f, 0.0f),    vec2(0.5f, 0.5f)},   //22
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(1.0f, 0.0f, 0.0f),    vec2(0.5f, 0.0f)},   //23
   }; 

   vector<u32> aabb_vertex_indices = 
   { 
      0, 1, 2, 2, 3, 0,          // bottom face
      16, 17, 18, 19, 18, 17,    // left face
      12, 13, 14, 14, 15, 12,    // back face
      20, 21, 22, 22, 23, 20,    // right face
      8, 9, 10, 11, 10, 9,       // front face
      4, 5, 6, 6, 7, 4           // top face
   };

   auto aabb_mesh                = new Mesh();
   aabb_mesh->name               = "aabb";
   aabb_mesh->vertices           = aabb_vertex_vec;
   aabb_mesh->indices            = aabb_vertex_indices;
   aabb_mesh->render_method      = GL_TRIANGLES;
   aabb_mesh->setup_gl_data();
   Geometry_Catalogue.insert({aabb_mesh->name, aabb_mesh});

   auto world_cell_mesh             = new Mesh();
   world_cell_mesh->name            = "world cell";
   world_cell_mesh->vertices        = aabb_vertex_vec;
   world_cell_mesh->indices         = aabb_vertex_indices;
   world_cell_mesh->render_method   = GL_TRIANGLES;
   world_cell_mesh->setup_gl_data();
   Geometry_Catalogue.insert({world_cell_mesh->name, world_cell_mesh});

   // SLOPE
   // with Z coming at the screen, X to the right, slope starts at x=0 high and goes low on x=1
   vector<Vertex> slope_vertex_vec = {
      // bottom
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.5f, 0.5f)},   //0
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 0.5f)},   //1
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 1.0f)},   //2
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.5f, 1.0f)},   //3
      // right   
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.5f, 0.5f, 0.0f),    vec2(1.0f, 0.5f)},   //4
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.5f, 0.5f, 0.0f),    vec2(0.5f, 0.5f)},   //5
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(0.5f, 0.5f, 0.0f),    vec2(1.0f, 1.0f)},   //6
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(0.5f, 0.5f, 0.0f),    vec2(0.5f, 1.0f)},   //7
      // front       
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.0f, 0.0f)},   //8
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.5f, 0.0f)},   //9
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.0f, 0.5f)},   //10
      // back
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),   vec2(0.0f, 0.0f)},   //11
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),   vec2(0.0f, 0.5f)},   //12
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),   vec2(0.5f, 0.0f)},   //13
      // left
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.0f, 0.0f)},   //14
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.5f, 0.0f)},   //15
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.0f, 0.5f)},   //16
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.5f, 0.5f)},   //17
   };

   vector<u32> slope_vertex_indices = 
   { 
      0, 1, 2, 2, 3, 0,          // bottom face
      8, 9, 10,                  // front
      11, 12, 13,                // back
      14, 15, 16, 17, 16, 15,    // left face
      4, 5, 6, 6, 7, 4           // right face (slope)
   };

   auto slope_mesh               = new Mesh();
   slope_mesh->name              = "slope";
   slope_mesh->vertices          = slope_vertex_vec;
   slope_mesh->indices           = slope_vertex_indices;
   slope_mesh->render_method     = GL_TRIANGLES;
   slope_mesh->setup_gl_data();
   Geometry_Catalogue.insert({slope_mesh->name, slope_mesh});

   // PLANE VBO
   vector<Vertex> planeVertices = {
      // positions            // normals         // texcoords
      Vertex{vec3{25.0f, -0.5f, 25.0f},   vec3{0.0f, 1.0f, 0.0f},   vec2{25.0f, 0.0f}},
      Vertex{vec3{-25.0f, -0.5f, 25.0f},  vec3{0.0f, 1.0f, 0.0f},   vec2{0.0f, 0.0f}},
      Vertex{vec3{-25.0f, -0.5f, -25.0f}, vec3{0.0f, 1.0f, 0.0f},   vec2{0.0f, 25.0f}},

      Vertex{vec3{25.0f, -0.5f,  25.0f},  vec3{0.0f, 1.0f, 0.0f},  vec2{25.0f,  0.0f}},
      Vertex{vec3{-25.0f, -0.5f, -25.0f}, vec3{0.0f, 1.0f, 0.0f},  vec2{ 0.0f, 25.0f}},
      Vertex{vec3{25.0f, -0.5f, -25.0f},  vec3{0.0f, 1.0f, 0.0f},  vec2{25.0f, 10.0f}}
   };
   auto plane_mesh               = new Mesh();
   plane_mesh->name              = "plane";
   plane_mesh->vertices          = planeVertices;
   plane_mesh->indices           = {0, 1, 2, 3, 4, 5};
   plane_mesh->render_method     = GL_TRIANGLES;
   plane_mesh->setup_gl_data();
   Geometry_Catalogue.insert({plane_mesh->name, plane_mesh});


   // QUAD VBO
   vector<Vertex> quad_vertex_vec = {
      Vertex{vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 0.0f)},
      Vertex{vec3(1.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 0.0f)},
      Vertex{vec3(1.0f, 1.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 1.0f)},
      Vertex{vec3(0.0f, 1.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 1.0f)}
   };
   // QUAD EBO
   vector<u32> quad_vertex_indices = { 0, 1, 2, 2, 3, 0 };


   // LINE (position is updated directly into VBO)
   /*vector<Vertex> line_vertex_vec = {
         Vertex{vec3(1, 1, 0)},
         Vertex{vec3(1, 1, 1)},
         Vertex{vec3(0, 1, 1)},
         Vertex{vec3(0, 1, 0)}
   };*/

   Mesh* quad_mesh            = new Mesh();
   quad_mesh->name            = "quad";
   quad_mesh->vertices        = quad_vertex_vec;
   quad_mesh->indices         = quad_vertex_indices;
   quad_mesh->render_method   = GL_TRIANGLES;
   quad_mesh->setup_gl_data();
   Geometry_Catalogue.insert({quad_mesh->name, quad_mesh});

 // QUAD HORIZONTAL
   vector<Vertex> quad_horizontal_vertex_vec = {
      Vertex{vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 0.0f)},
      Vertex{vec3(1.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 0.0f)},
      Vertex{vec3(1.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 1.0f)},
      Vertex{vec3(0.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 1.0f)}
   };

   Mesh* quad_horizontal_mesh             = new Mesh();
   quad_horizontal_mesh->name             = "quad_horizontal";
   quad_horizontal_mesh->vertices         = quad_horizontal_vertex_vec;
   quad_horizontal_mesh->indices          = quad_vertex_indices;
   quad_horizontal_mesh->render_method    = GL_TRIANGLES;
   quad_horizontal_mesh->setup_gl_data();
   Geometry_Catalogue.insert({quad_horizontal_mesh->name, quad_horizontal_mesh});

   // TRIGGER
   auto trigger_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "player_cylinder");
   Geometry_Catalogue.insert({trigger_mesh->name, trigger_mesh});

   // PLAYER CAPSULE
   load_wavefront_obj_as_mesh(MODELS_PATH, "capsule");

   // LIGHTBULB
   auto lightbulb_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "lightbulb");
}


void initialize_shaders() 
{
   // text shader
	auto text_shader = create_shader_program("text", "vertex_text", "fragment_text");
   text_shader->use();
	text_shader->setMatrix4("projection", glm::ortho(0.0f, G_DISPLAY_INFO.VIEWPORT_WIDTH, 0.0f, G_DISPLAY_INFO.VIEWPORT_HEIGHT));
   Shader_Catalogue.insert({text_shader->name, text_shader});

   // general model shader
   auto model_shader = create_shader_program("model", "vertex_model", "fragment_multiple_lights");
   Shader_Catalogue.insert({model_shader->name, model_shader});

   // tiled texture model shader
   auto tiled_texture_model_shader = create_shader_program("tiledTextureModel", "vertex_model", "fragment_tiled_texture_model_shader");
   Shader_Catalogue.insert({tiled_texture_model_shader->name, tiled_texture_model_shader});

   // draw line shader
	auto line_shader = create_shader_program("line", "vertex_debug_line", "fragment_debug_line");
   Shader_Catalogue.insert({line_shader->name, line_shader});

   // immediate draw shaders
   auto im_point_shader = create_shader_program("immediate_point", "vertex_point", "fragment_point");
   Shader_Catalogue.insert({im_point_shader->name, im_point_shader});

   // immediate draw mesh shaders
   auto im_mesh_shader = create_shader_program("im_mesh", "vertex_simple_mesh", "fragment_color");
   Shader_Catalogue.insert({im_mesh_shader->name, im_mesh_shader});

   // editor entity shaders
   auto ortho_shader = create_shader_program("ortho_gui", "vertex_static", "fragment_static");
   Shader_Catalogue.insert({ortho_shader->name, ortho_shader});

   // general model shader
   auto color_shader = create_shader_program("color", "vertex_model", "fragment_color");
   Shader_Catalogue.insert({color_shader->name, color_shader});

   // depth map shader
   auto depth_shader = create_shader_program("depth", "vertex_depth", "fragment_empty");
   Shader_Catalogue.insert({depth_shader->name, depth_shader});

   // depth map debug shader
   auto depth_debug_shader = create_shader_program("depth_debug", "vertex_depth_debug", "fragment_depth_debug");
   Shader_Catalogue.insert({depth_debug_shader->name, depth_debug_shader});

   // depth cubemap
   auto depth_cubemap_shader = create_shader_program(
      "depth_cubemap", "vertex_depth_cubemap", "geometry_depth_cubemap" ,"fragment_depth_cubemap"
   );
   Shader_Catalogue.insert({depth_cubemap_shader->name, depth_cubemap_shader});

   // editor entity arrow shader
   auto editor_arrow_shader = create_shader_program(
      "ed_entity_arrow_shader", "vertex_model", "fragment_ed_entity_arrow"
   );
   Shader_Catalogue.insert({editor_arrow_shader->name, editor_arrow_shader});
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
	G_DISPLAY_INFO.window = glfwCreateWindow(G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT, "Ravenous", NULL, NULL);
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
	glViewport(0, 0, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT);
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
      player->entity_ptr->render_me = false;
      glfwSetInputMode(G_DISPLAY_INFO.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      Editor::end_dear_imgui_frame();

      G_BUFFERS.rm_buffer->add("Game Mode", 2000);
   }
   else if(PROGRAM_MODE.current == GAME_MODE)
   {
      PROGRAM_MODE.last    = PROGRAM_MODE.current;
      PROGRAM_MODE.current = EDITOR_MODE;
      G_SCENE_INFO.camera  = G_SCENE_INFO.views[0];
      player->entity_ptr->render_me = true;
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