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
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <limits>
#include <assert.h>
#include <algorithm>
#include <stdint.h>

// TYPE DEFINITIONS
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long u64;
const float PI = 3.141592;
typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::mat4 mat4;
typedef std::string string;

const float MAX_FLOAT = std::numeric_limits<float>::max();
const string PROJECT_PATH = "c:/repositories/ravenous";
const string TEXTURES_PATH = PROJECT_PATH + "/assets/textures/";
const string MODELS_PATH = PROJECT_PATH + "/assets/models/";
const string FONTS_PATH = PROJECT_PATH + "/assets/fonts/";
const string SHADERS_FOLDER_PATH = PROJECT_PATH + "/shaders/";
const string CAMERA_FILE_PATH = PROJECT_PATH + "/camera.txt";
const string SCENES_FOLDER_PATH = PROJECT_PATH + "/scenes/";
const string SHADERS_FILE_EXTENSION = ".shd";
const string CONFIG_FILE_PATH = PROJECT_PATH + "/config.txt";

// PLAYER CYLINDER SETTINGS ... !!!
// 1.75m of height
float CYLINDER_HALF_HEIGHT = 0.875; 
float CYLINDER_RADIUS = 0.20f;

float VEC_COMPARE_PRECISION = 0.00001f;

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
   double left_click_x;
   double left_click_y;
   double x;
   double y;
};

struct GlobalInputInfo {
   bool forget_last_mouse_coords = true;
   MouseCoordinates mouse_coords;
   u64 key_state = 0;
   u8 mouse_state = 0;
} G_INPUT_INFO;

struct GlobalFrameInfo {
   float duration;
   float last_frame_time;
   int frame_counter;
   float current_fps;
   u16 frame_counter_3 = 0;
   u16 frame_counter_10 = 0;
   float time_step = 1;
} G_FRAME_INFO;

struct ProgramConfig {
   string initial_scene;
} G_CONFIG;

// SOURCE INCLUDES
#include <mesh.h>
#include <utils.h>
#include <character.h>
#include <shader.h>
#include <entities.h>
#include <model.h>
#include <player.h>
#include <camera.h>
#include <parser.h>
#include <world.h>
#include <globals.h>
#include <entity_manager.h>

// entity manager
EntityManager Entity_Manager;

bool is_vec2_equal(vec2 vec1, vec2 vec2);

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "ravenous_imconfig.h"

bool save_configs_to_file();
bool is_float_zero(float x);
void toggle_program_modes(Player* player);
void erase_entity(Scene* scene, Entity* entity);

#include <loaders.h>
#include <raycast.h>
#include <render.h>
#include <input.h>
#include <collision.h>
#include <scene.h>
#include <console.h>
#include <editor/editor_main.h>
#include <gameplay.h>

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
EntityBuffer* allocate_entity_buffer(size_t size);
RenderMessageBuffer* allocate_render_message_buffer(size_t size);
void update_buffers(Player* player, bool changed_cells);
void start_frame();
ProgramConfig load_configs();
void check_all_entities_have_shaders();
void setup_gl();

int main()
{
   // INITIAL GLFW AND GLAD SETUPS
	setup_GLFW(true);
   setup_gl();

   // populates texture catalogue with diffuse textures
   load_textures_from_assets_folder();

   // create cameras
	Camera* editor_camera = new Camera();
   Camera* first_person_camera = new Camera();
   G_SCENE_INFO.views[0] = editor_camera;
   G_SCENE_INFO.views[1] = first_person_camera;

	// LOAD SHADERS AND GEOMETRY
	load_text_textures("Consola.ttf", 12);
   initialize_shaders();
   create_boilerplate_geometry();

   // Allocate buffers
   EntityBuffer* entity_buffer = allocate_entity_buffer(WORLD_CELL_CAPACITY * 8);
   G_BUFFERS.entity_buffer = entity_buffer;
   RenderMessageBuffer* render_message_buffer = allocate_render_message_buffer(10);
   G_BUFFERS.rm_buffer = render_message_buffer;
   initialize_console_buffers();

   // loads initial scene
   G_CONFIG = load_configs();
   load_scene_from_file(G_CONFIG.initial_scene, &World);
   Entity_Manager.set_default_entity_attributes(         // sets some loaded assets from scene as
      "aabb", "model", "sandstone"                       // defaults for entity construction
   );  
   Player* player = G_SCENE_INFO.player;
   World.update_entity_world_cells(player->entity_ptr);  // sets player to the world
   update_buffers(player, true);                         // populates collision buffer and others
   
   Editor::initialize();

   // Pre-loop checks
   check_all_entities_have_shaders();

	// MAIN LOOP
	while (!glfwWindowShouldClose(G_DISPLAY_INFO.window))
	{
      // -------------
      // START FRAME
      // -------------
		start_frame();

      // -------------
		//	INPUT PHASE
      // -------------
      auto input_flags = input_phase();

      switch(PROGRAM_MODE.current)
      {
         case CONSOLE_MODE:
            handle_console_input(input_flags, player, &World, G_SCENE_INFO.camera);
            break;
         case EDITOR_MODE:
            Editor::start_frame();
            Editor::handle_input_flags(input_flags, player);
            if(!ImGui::GetIO().WantCaptureKeyboard)
               handle_common_input(input_flags, player);
            break;
         case GAME_MODE:
            game_handle_input(input_flags, player);
            handle_common_input(input_flags, player);
            break;
      }
      reset_input_flags(input_flags);

      // -------------
		//	UPDATE PHASE
      // -------------
		camera_update(G_SCENE_INFO.camera, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT, player);
      move_player(player);
      bool changed_cells = update_player_world_cells(player);
      update_buffers(player, changed_cells);
      update_player_state(player, &World);
		update_scene_objects();

      // -------------
		//	RENDER PHASE
      // -------------
		glClearColor(0.196, 0.298, 0.3607, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_scene(G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
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
      render_immediate(&G_IMMEDIATE_DRAW, G_SCENE_INFO.camera);
      render_message_buffer_contents();

      // -------------
      // FINISH FRAME
      // -------------
      Entity_Manager.safe_delete_marked_entities();
		glfwSwapBuffers(G_DISPLAY_INFO.window);
      if(PROGRAM_MODE.current == EDITOR_MODE) Editor::end_frame();
	}

	glfwTerminate();
	return 0;
}

void start_frame()
{
   float current_frame_time = glfwGetTime();
   G_FRAME_INFO.duration = current_frame_time - G_FRAME_INFO.last_frame_time;
   G_FRAME_INFO.last_frame_time = current_frame_time;
   if(G_FRAME_INFO.duration > 0.02)
   {
      G_FRAME_INFO.duration = 0.02;
      //std::cout << "delta time exceeded.\n";
   } 
   G_FRAME_INFO.current_fps = 1.0f / G_FRAME_INFO.duration;
   G_FRAME_INFO.frame_counter_3 = ++G_FRAME_INFO.frame_counter_3 % 3;
   G_FRAME_INFO.frame_counter_10 = ++G_FRAME_INFO.frame_counter_10 % 10;
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

EntityBuffer* allocate_entity_buffer(size_t size)
{
   auto e_buffer = new EntityBuffer;
   e_buffer->buffer = new EntityBufferElement[size];
   e_buffer->size = size;
   return e_buffer;
}

RenderMessageBuffer* allocate_render_message_buffer(size_t size)
{
   auto rm_buffer = new RenderMessageBuffer;
   rm_buffer->buffer = new RenderMessageBufferElement[size];
   rm_buffer->size = size;
   return rm_buffer;
}

void update_buffers(Player* player, bool changed_cells)
{
   // UPDATE COLLISION DETECTION ENTITY BUFFER
   {
      // copies collision-check-relevant entity ptrs to a buffer
      // with metadata about the collision check for the entity
      auto buffer = G_BUFFERS.entity_buffer->buffer;     
      if(changed_cells)
      {
         int new_size = 0;
         for(int i = 0; i < player->entity_ptr->world_cells_count; i++)
         {
            auto cell = player->entity_ptr->world_cells[i];
            for(int j = 0; j < cell->count; j++)
            {
               auto entity = cell->entities[j];
               buffer->entity = entity;
               buffer->collision_check = false;
               buffer++;
               new_size++;
            }
         }
         G_BUFFERS.entity_buffer->size = new_size;
      }
      else
      {
         for(int i = 0; i < G_BUFFERS.entity_buffer->size; i++)
         {
            buffer->collision_check = false;
            buffer++;
         }
      }
   }

   // update render message buffer
   {
      size_t size = G_BUFFERS.rm_buffer->size;
      auto item = G_BUFFERS.rm_buffer->buffer;
      for(int i = 0; i < size; i++)
      {
         if(item->message != "" && item->elapsed >= item->duration)
         {
            item->message = "";
            G_BUFFERS.rm_buffer->count -= 1;
         }
         item++;
      }
   }
}

ProgramConfig load_configs()
{
   ifstream reader(CONFIG_FILE_PATH);

   if(!reader.is_open())
   {
      cout << "FATAL: Cant load config file '" + CONFIG_FILE_PATH + "', path NOT FOUND \n";  
      assert(false);
   }

   auto config = ProgramConfig();

   // starts reading
   std::string line;
   Parser::Parse p;
   int line_count = 0;

   // parses entity
   while(parser_nextline(&reader, &line, &p))
   {
      line_count++;
      p = parse_token(p);
      string attribute = p.string_buffer;

      p = parse_all_whitespace(p);
      p = parse_symbol(p);
      if(p.cToken != '=')
      {
         std::cout << 
            "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << 
            CONFIG_FILE_PATH  << 
            "') LINE NUMBER " << 
            line_count << "\n";
            
         assert(false);
      }
      
      p = parse_all_whitespace(p);
      p = parse_token(p);
      string value = p.string_buffer;

      if(attribute == "scene")
      {
         config.initial_scene = value;
      }

   }

   return config;
}

bool save_configs_to_file()
{

   ofstream writer(CONFIG_FILE_PATH);
   if(!writer.is_open())
   {
      cout << "Saving config file failed.\n";
      return false;
   }

   writer << "scene = " << G_CONFIG.initial_scene << "\n";

   writer.close();
   cout << "Config file saved succesfully.\n";

   return true;
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
      Vertex{vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.0f, 0.0f)},   //0
      Vertex{vec3(1.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.5f, 0.0f)},   //1
      Vertex{vec3(1.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.5f, 0.5f)},   //2
      Vertex{vec3(0.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.0f, 0.5f)},   //3
      // top
      Vertex{vec3(0.0f, 1.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 0.0f)},    //4
      Vertex{vec3(1.0f, 1.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.5f, 0.0f)},    //5
      Vertex{vec3(1.0f, 1.0f, 1.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.5f, 0.5f)},    //6
      Vertex{vec3(0.0f, 1.0f, 1.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 0.5f)}     //7
   };

   vector<u32> aabb_vertex_indices = 
   { 
      0, 1, 2, 2, 3, 0,    // bottom face
      0, 4, 1, 1, 5, 4,    // side face 1
      1, 5, 2, 2, 6, 5,    // back face
      2, 6, 3, 3, 7, 6,    // side face 2
      3, 7, 0, 0, 4, 7,    // front face
      4, 5, 6, 6, 7, 4     // top face
   };

   auto aabb_mesh = new Mesh();
   aabb_mesh->name = "aabb";
   aabb_mesh->vertices = aabb_vertex_vec;
   aabb_mesh->indices = aabb_vertex_indices;
   aabb_mesh->render_method = GL_TRIANGLES;
   aabb_mesh->setup_gl_data();
   Geometry_Catalogue.insert({aabb_mesh->name, aabb_mesh});

   auto world_cell_mesh = new Mesh();
   world_cell_mesh->name = "world cell";
   world_cell_mesh->vertices = aabb_vertex_vec;
   world_cell_mesh->indices = aabb_vertex_indices;
   world_cell_mesh->render_method = GL_TRIANGLES;
   world_cell_mesh->setup_gl_data();
   Geometry_Catalogue.insert({world_cell_mesh->name, world_cell_mesh});


   // SLOPE
   vector<Vertex> slope_vertex_vec = {
      // bottom
      Vertex{vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.0f, 0.0f)},   //0
      Vertex{vec3(1.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.5f, 0.0f)},   //1
      Vertex{vec3(1.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.5f, 0.5f)},   //2
      Vertex{vec3(0.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.0f, 0.5f)},   //3
      // top
      Vertex{vec3(0.0f, 1.0f, 0.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.0f, 0.0f)},   //4
      Vertex{vec3(0.0f, 1.0f, 1.0f),vec3(0.0f, 0.0f, -1.0f),vec2(0.5f, 0.0f)},   //5
   };

   vector<u32> slope_vertex_indices = 
   { 
      0, 1, 2, 2, 3, 0,    // bottom face
      0, 4, 1,             // side face 1
      3, 5, 2,             // side face 2
      0, 4, 3, 3, 5, 4,    // back face
      4, 1, 5, 5, 2, 1,    // slope face
   };

   auto slope_mesh = new Mesh();
   slope_mesh->name = "slope";
   slope_mesh->vertices = slope_vertex_vec;
   slope_mesh->indices = slope_vertex_indices;
   slope_mesh->render_method = GL_TRIANGLES;
   slope_mesh->setup_gl_data();
   Geometry_Catalogue.insert({slope_mesh->name, slope_mesh});


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

   Mesh* quad_mesh = new Mesh();
   quad_mesh->name = "quad";
   quad_mesh->vertices = quad_vertex_vec;
   quad_mesh->indices = quad_vertex_indices;
   quad_mesh->render_method = GL_TRIANGLES;
   quad_mesh->setup_gl_data();
   Geometry_Catalogue.insert({quad_mesh->name, quad_mesh});

 // QUAD HORIZONTAL
   vector<Vertex> quad_horizontal_vertex_vec = {
      Vertex{vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 0.0f)},
      Vertex{vec3(1.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 0.0f)},
      Vertex{vec3(1.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 1.0f)},
      Vertex{vec3(0.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 1.0f)}
   };

   Mesh* quad_horizontal_mesh = new Mesh();
   quad_horizontal_mesh->name = "quad_horizontal";
   quad_horizontal_mesh->vertices = quad_horizontal_vertex_vec;
   quad_horizontal_mesh->indices = quad_vertex_indices;
   quad_horizontal_mesh->render_method = GL_TRIANGLES;
   quad_horizontal_mesh->setup_gl_data();
   Geometry_Catalogue.insert({quad_horizontal_mesh->name, quad_horizontal_mesh});


   // PLAYER CYLINDER
   Mesh* cylinder_mesh = new Mesh();
   cylinder_mesh->name = "player_cylinder";
   cylinder_mesh->vertices = construct_cylinder(CYLINDER_RADIUS, CYLINDER_HALF_HEIGHT, 24);
   cylinder_mesh->render_method = GL_TRIANGLE_STRIP;
   cylinder_mesh->setup_gl_data();
   Geometry_Catalogue.insert({cylinder_mesh->name, cylinder_mesh});
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

   // draw line shader
	auto line_shader = create_shader_program("line", "vertex_debug_line", "fragment_debug_line");
   Shader_Catalogue.insert({line_shader->name, line_shader});

   // immediate draw shaders
   auto im_point_shader = create_shader_program("immediate_point", "vertex_point", "fragment_point");
   Shader_Catalogue.insert({im_point_shader->name, im_point_shader});

   // editor entity shaders
   auto static_shader = create_shader_program("static", "vertex_static", "fragment_static");
   Shader_Catalogue.insert({static_shader->name, static_shader});

   // general model shader
   auto color_shader = create_shader_program("color", "vertex_model", "fragment_color");
   Shader_Catalogue.insert({color_shader->name, color_shader});
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
	}
}

bool is_vec2_equal(vec2 vec1, vec2 vec2)
{
   float x_diff = abs(vec1.x - vec2.x);
   float y_diff = abs(vec1.y - vec2.y);
   return x_diff < VEC_COMPARE_PRECISION && y_diff < VEC_COMPARE_PRECISION;
}

void setup_GLFW(bool debug) {
	// Setup the window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
   if(PROGRAM_MODE.current == EDITOR_MODE)
   {
      PROGRAM_MODE.last    = PROGRAM_MODE.current;
      PROGRAM_MODE.current = GAME_MODE;
      G_SCENE_INFO.camera  = G_SCENE_INFO.views[1];
      player->entity_ptr->render_me = false;
      glfwSetInputMode(G_DISPLAY_INFO.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      Editor::end_frame();

      G_BUFFERS.rm_buffer->add("Game Mode", 2000);
   }
   else if(PROGRAM_MODE.current == GAME_MODE)
   {
      PROGRAM_MODE.last    = PROGRAM_MODE.current;
      PROGRAM_MODE.current = EDITOR_MODE;
      G_SCENE_INFO.camera  = G_SCENE_INFO.views[0];
      player->entity_ptr->render_me = true;
      glfwSetInputMode(G_DISPLAY_INFO.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      Editor::start_frame();

      G_BUFFERS.rm_buffer->add("Editor Mode", 2000);
   }
}

inline bool is_float_zero(float x)
{
   return abs(x) < 0.0001;
}

void setup_gl()
{
	glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glEnable(GL_PROGRAM_POINT_SIZE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
