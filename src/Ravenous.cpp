// DEPENDENCY INCLUDES
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
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
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

// PLAYER CYLINDER SETTINGS ... !!!
float CYLINDER_HALF_HEIGHT = 0.35f;
float CYLINDER_RADIUS = 0.10f;

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
   const float VIEWPORT_WIDTH = 1200;
   const float VIEWPORT_HEIGHT = 900;
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
   float delta_time;
   float last_frame_time;
   int frame_counter;
   float current_fps;
   u16 frame_counter_3 = 0;
   u16 frame_counter_10 = 0;
   float time_step = 1;
} G_FRAME_INFO;

#include <mesh.h>

void print_vec(vec3 vec, std::string prefix)
{
   std::cout << prefix << ": (" << vec.x << ", " << vec.y << ", " << vec.z << ") \n";
}

void print_vertex_array_position(Vertex* vertex, size_t length, std::string title)
{
   std::cout << title << "\n";
   for(int i = 0; i < length; i++)
   {
      vec3 pos = vertex[i].position;
      std::cout << "[" << i << "] : (" << pos.x << ", " << pos.y << ", " << pos.z << ") \n";
   }
}

void print_vec_every_3rd_frame(vec3 vec, std::string prefix)
{
   if(G_FRAME_INFO.frame_counter_3 == 0)
      print_vec(vec, prefix);
}

void print_every_3rd_frame(std::string thing, std::string prefix)
{
   if(G_FRAME_INFO.frame_counter_3 == 0)
      std::cout << prefix << ": " << thing << "\n";
}


// SOURCE INCLUDES
#include <text.h>
#include <shader.h>
#include <model.h>
#include <camera.h>
#include <entities.h>
#include <player.h>
#include <parser.h>
#include <loaders.h>

// catalogues 
std::map<string, Mesh*> Geometry_Catalogue;
std::map<string, Shader*> Shader_Catalogue;
std::map<string, Texture> Texture_Catalogue;

struct GlobalImmediateDraw {
   Mesh* meshes[10];
   int  ids[10];
   int ind = 0;
   void add(int id, vec3* triangles, int n = 1)
   {
      // build vertex vector
      vector<Vertex> vertex_vec;
      for (int i = 0; i < n; i++)
         vertex_vec.push_back(Vertex{triangles[i]});

      // if present, update positions only
      for(int i = 0; i < ind; i ++)
         if (ids[i] == id)
         {
            meshes[i]->vertices = vertex_vec;
            return;
         }

      // if new, create new mesh      
      auto mesh = new Mesh();
      mesh->vertices = vertex_vec;
      if (n == 1)
         mesh->render_method = GL_POINTS;
      else
         mesh->render_method = GL_TRIANGLE_STRIP;
      mesh->gl_data = setup_gl_data_for_mesh(mesh);
      ids[ind] = id;
      meshes[ind++] = mesh;
   };
} G_IMMEDIATE_DRAW;

#include <render.h>

// GLOBAL STRUCT VARIABLES (WITH CUSTOM TYPES)
GlobalEntityInfo G_ENTITY_INFO;

struct GlobalSceneInfo {
   Scene* active_scene = NULL;
   Camera* camera;
   Camera* views[2];
   Player* player;
   bool input_mode = false;
   string scene_name;
} G_SCENE_INFO;

struct EntityBufferElement {
   Entity* entity;
   bool  collision_check = false;
};

struct EntityBuffer {
   EntityBufferElement* buffer;
   size_t size;
};

struct GlobalBuffers {
   void* buffers[20];
} G_BUFFERS;

bool compare_vec2(vec2 vec1, vec2 vec2);



#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <input.h>
#include <collision.h>
#include <scene.h>
#include <console.h>
#include <raycast.h>
#include <editor.h>
#include <gameplay.h>

#define glCheckError() glCheckError_(__FILE__, __LINE__) 

// OPENGL OBJECTS
unsigned int texture, texture_specular;

using namespace glm;

// FUNCTION PROTOTYPES
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void setup_window(bool debug);
void render_ray();
void update_scene_objects();
void initialize_shaders();
void create_boilerplate_geometry();
void render_text_overlay(Camera* camera, Player* player);
GLenum glCheckError_(const char* file, int line);
std::string format_float_tostr(float num, int precision);
EntityBuffer* allocate_entity_buffer(size_t size);
void update_buffers();
void check_view_mode(Player* player);
void start_frame();


int main() 
{
   // reads from camera position file
   float* camera_pos = load_camera_settings(CAMERA_FILE_PATH);

	Camera* editor_camera = camera_create(
      vec3(camera_pos[0], camera_pos[1], camera_pos[2]), vec3(camera_pos[3], camera_pos[4], camera_pos[5]), false
   );
   Camera* first_person_camera = new Camera();

	G_SCENE_INFO.camera = editor_camera;
   G_SCENE_INFO.views[0] = editor_camera;
   G_SCENE_INFO.views[1] = first_person_camera;


	// INITIAL GLFW AND GLAD SETUPS
	setup_window(true);

	// gl enables
	glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glEnable(GL_PROGRAM_POINT_SIZE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// SHADERS
	load_text_textures("Consola.ttf", 12);
   initialize_shaders();
   create_boilerplate_geometry();

   load_scene_from_file("test");

   Player* player = G_SCENE_INFO.player;

   // Allocate buffers
   EntityBuffer* entity_buffer = allocate_entity_buffer(50);
   G_BUFFERS.buffers[0] = entity_buffer;
   initialize_console_buffers();

   Editor::initialize();

	// MAIN LOOP
	while (!glfwWindowShouldClose(G_DISPLAY_INFO.window))
	{
      // START FRAME
		start_frame();

		//	INPUT PHASE
      auto input_flags = input_phase();

      switch(PROGRAM_MODE.current)
      {
         case CONSOLE_MODE:
         {
            handle_console_input(input_flags, player);
            break;
         }
         case EDITOR_MODE:
         {
            handle_input_flags(input_flags, player);
            Editor::start_frame();
            break;
         }
         default:
         {
            handle_input_flags(input_flags, player);
         }
      }

		//	UPDATE PHASE
      check_view_mode(player);
		camera_update(G_SCENE_INFO.camera, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT);
      update_buffers();
      update_player_state(player);
		update_scene_objects();

		//	RENDER PHASE
		glClearColor(0.196, 0.298, 0.3607, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_scene(G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
      switch(PROGRAM_MODE.current)
      {
         case CONSOLE_MODE:
         {
            render_console();
            break;
         }
         case EDITOR_MODE:
         {
            Editor::render();
            render_text_overlay(G_SCENE_INFO.camera, player);
            break;
         }
      }
      Editor::debug_entities();
      render_immediate(&G_IMMEDIATE_DRAW, G_SCENE_INFO.camera);

      // FINISH FRAME
		glfwSwapBuffers(G_DISPLAY_INFO.window);
      if(PROGRAM_MODE.current == EDITOR_MODE) Editor::end_frame();
	}

	glfwTerminate();
	return 0;
}

void start_frame()
{
   float current_frame_time = glfwGetTime();
   G_FRAME_INFO.delta_time = current_frame_time - G_FRAME_INFO.last_frame_time;
   G_FRAME_INFO.last_frame_time = current_frame_time;
   if(G_FRAME_INFO.delta_time > 0.02)
   {
      G_FRAME_INFO.delta_time = 0.02;
      //std::cout << "delta time exceeded.\n";
   } 
   G_FRAME_INFO.current_fps = 1.0f / G_FRAME_INFO.delta_time;
   G_FRAME_INFO.frame_counter_3 = ++G_FRAME_INFO.frame_counter_3 % 3;
   G_FRAME_INFO.frame_counter_10 = ++G_FRAME_INFO.frame_counter_10 % 10;
}


void check_view_mode(Player* player)
{
   if(PROGRAM_MODE.current == EDITOR_MODE)
   {
      // do nothing
   }
   else if(PROGRAM_MODE.current == GAME_MODE)
   {
      // sets camera to player's eye position
      G_SCENE_INFO.camera->Position = player->entity_ptr->position;
      G_SCENE_INFO.camera->Position.y += player->half_height * 2.0 / 3.0; 
   }
}


EntityBuffer* allocate_entity_buffer(size_t size)
{
   EntityBuffer* e_buffer = new EntityBuffer;
   e_buffer->buffer = new EntityBufferElement[size];
   e_buffer->size = size;
   return e_buffer;
}

void update_buffers()
{
   // UPDATE COLLISION DETECTION ENTITY BUFFER
   {
      // copies from active_scene entity list, all entity pointers to a buffer with metadata about the collision check for the entity
      Entity** entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
      size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();            

      auto entity_buffer = (EntityBuffer*)G_BUFFERS.buffers[0];
      EntityBufferElement* entity_buf_iter = entity_buffer->buffer;       
      for(int i = 0; i < entity_list_size; ++i) // ASSUMES that entity_list_size is ALWYAS smaller then the EntityBuffer->size    
      {
         entity_buf_iter->entity = *entity_iterator;
         entity_buf_iter->collision_check = false;
         entity_buf_iter++; 
         entity_iterator++;
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

   // GEOMETRY

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
   aabb_mesh->gl_data = setup_gl_data_for_mesh(aabb_mesh);
   Geometry_Catalogue.insert({aabb_mesh->name, aabb_mesh});

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
   slope_mesh->gl_data = setup_gl_data_for_mesh(slope_mesh);
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
   quad_mesh->gl_data = setup_gl_data_for_mesh(quad_mesh);
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
   quad_horizontal_mesh->gl_data = setup_gl_data_for_mesh(quad_horizontal_mesh);
   Geometry_Catalogue.insert({quad_horizontal_mesh->name, quad_horizontal_mesh});


   // PLAYER CYLINDER
   Mesh* cylinder_mesh = new Mesh();
   cylinder_mesh->name = "player_cylinder";
   cylinder_mesh->vertices = construct_cylinder(CYLINDER_RADIUS, CYLINDER_HALF_HEIGHT, 24);
   cylinder_mesh->render_method = GL_TRIANGLE_STRIP;
   cylinder_mesh->gl_data = setup_gl_data_for_mesh(cylinder_mesh);
   Geometry_Catalogue.insert({cylinder_mesh->name, cylinder_mesh});
}

void render_text_overlay(Camera* camera, Player* player) 
{
   // render info text
   float GUI_x = 25;
   float GUI_y = G_DISPLAY_INFO.VIEWPORT_HEIGHT - 60;

   string player_floor = "player floor: ";
   if(player->standing_entity_ptr != NULL)
   {
      player_floor += player->standing_entity_ptr->name;
   }

   string GUI_atts[]{
      format_float_tostr(camera->Position.x, 2),               //0
      format_float_tostr(camera->Position.y,2),                //1
      format_float_tostr(camera->Position.z,2),                //2
      format_float_tostr(camera->Pitch,2),                     //3
      format_float_tostr(camera->Yaw,2),                       //4
      format_float_tostr(camera->Front.x,2),                   //5
      format_float_tostr(camera->Front.y,2),                   //6
      format_float_tostr(camera->Front.z,2),                   //7
      format_float_tostr(player->entity_ptr->position.x,1),    //8
      format_float_tostr(player->entity_ptr->position.y,1),    //9 
      format_float_tostr(player->entity_ptr->position.z,1),    //10
      format_float_tostr(G_FRAME_INFO.time_step,1)             //11
   };
 
   string camera_position  = "camera:   x: " + GUI_atts[0] + " y:" + GUI_atts[1] + " z:" + GUI_atts[2];
   string camera_front     = "    dir:  x: " + GUI_atts[5] + " y:" + GUI_atts[6] + " z:" + GUI_atts[7];
   string mouse_stats      = "    pitch: " + GUI_atts[3] + " yaw: " + GUI_atts[4];
   string fps              = to_string(G_FRAME_INFO.current_fps);
   string fps_gui          = "FPS: " + fps.substr(0, fps.find('.', 0) + 2);
   string player_pos       = "player:   x: " +  GUI_atts[8] + " y: " +  GUI_atts[9] + " z: " +  GUI_atts[10];
   string time_step_string = "time step: " + GUI_atts[11] + "x";


   vec3 player_state_text_color;
   std::string player_state_text;
   switch(player->player_state)
   {
      case PLAYER_STATE_STANDING:
         player_state_text_color = vec3(0, 0.8, 0.1);
         player_state_text = "PLAYER STANDING";
         break;
      case PLAYER_STATE_FALLING:
         player_state_text_color = vec3(0.8, 0.1, 0.1);
         player_state_text = "PLAYER FALLING";
         break;
      case PLAYER_STATE_FALLING_FROM_EDGE:
         player_state_text_color = vec3(0.8, 0.1, 0.3);
         player_state_text = "PLAYER FALLING FROM EDGE";
         break;
      case PLAYER_STATE_JUMPING:
         player_state_text_color = vec3(0.1, 0.3, 0.8);
         player_state_text = "PLAYER JUMPING";
         break;
      case PLAYER_STATE_SLIDING:
         player_state_text_color = vec3(0.1, 0.3, 0.8);
         player_state_text = "PLAYER SLIDING";
         break;
      case PLAYER_STATE_SLIDE_FALLING:
         player_state_text_color = vec3(0.1, 0.3, 0.8);
         player_state_text = "PLAYER SLIDE FALLING";
         break;
      case PLAYER_STATE_EVICTED_FROM_SLOPE:
         player_state_text_color = vec3(0.1, 0.3, 0.8);
         player_state_text = "PLAYER EVICTED FROM SLOPE";
         break;
   }

   std::string view_mode_text;
   switch(PROGRAM_MODE.current)
   {
      case EDITOR_MODE:
         view_mode_text = "EDITOR MODE";
         break;
      case GAME_MODE:
         view_mode_text = "GAME MODE";
         break;
   }

   float scale = 1;
   render_text(camera_position,  GUI_x, GUI_y, scale);
   render_text(camera_front,     GUI_x, GUI_y - 25, scale);
   render_text(mouse_stats,      GUI_x, GUI_y - 50, scale);
   render_text(player_pos,       GUI_x, GUI_y - 75, scale);

   render_text(player_state_text,     GUI_x, 25, 1.4, player_state_text_color);
   render_text(view_mode_text,        GUI_x, 50, 1.4);
   render_text(player_floor,          GUI_x, 75, 1.4);

   render_text(G_SCENE_INFO.scene_name,   G_DISPLAY_INFO.VIEWPORT_WIDTH - 100, 75, scale);
   render_text(time_step_string,          G_DISPLAY_INFO.VIEWPORT_WIDTH - 130, 50, 1, vec3(0.8, 0.8, 0.2));
   render_text(fps_gui,                   G_DISPLAY_INFO.VIEWPORT_WIDTH - 100, 25, scale);
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
}


std::string format_float_tostr(float num, int precision) 
{
	string temp = std::to_string(num);
	return temp.substr(0, temp.find(".") + 3);
}


inline void update_scene_objects() 
{
	Entity** entity_iterator = &G_SCENE_INFO.active_scene->entities[0];
	size_t list_size = G_SCENE_INFO.active_scene->entities.size();
	for (int i = 0; i < list_size; i++) 
   {
      Entity* &entity = *entity_iterator;
		// Updates model matrix;	
		mat4 model = translate(mat4identity, entity->position);
		model = rotate(model, radians(entity->rotation.x), vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, radians(entity->rotation.z), vec3(0.0f, 0.0f, 1.0f));
		model = scale(model, entity->scale);
		entity->matModel = model;

      entity_iterator++;
	}
}

bool compare_vec2(vec2 vec1, vec2 vec2)
{
   float x_diff = abs(vec1.x - vec2.x);
   float y_diff = abs(vec1.y - vec2.y);
   return x_diff < VEC_COMPARE_PRECISION && y_diff < VEC_COMPARE_PRECISION;
}

void setup_window(bool debug) {
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
