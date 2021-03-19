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

// DEFINES
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long int u64;
const float PI = 3.141592;

const float MAX_FLOAT = std::numeric_limits<float>::max();
const std::string PROJECT_PATH = "c:/repositories/ravenous";
const std::string TEXTURES_PATH = PROJECT_PATH + "/assets/textures/";
const std::string MODELS_PATH = PROJECT_PATH + "/assets/models/";
const std::string FONTS_PATH = PROJECT_PATH + "/assets/fonts/";
const std::string SHADERS_FOLDER_PATH = PROJECT_PATH + "/shaders/";
const std::string SHADERS_FILE_EXTENSION = ".shd";

// PLAYER CYLINDER SETTINGS ... !!!
float CYLINDER_HALF_HEIGHT = 0.35f;
float CYLINDER_RADIUS = 0.10f;


const glm::mat4 mat4identity(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);


// GLOBAL STRUCT VARIABLES OR TYPES 
struct GLData {
   GLuint VAO = 0;
   GLuint VBO = 0;
   GLuint EBO = 0;
};

struct GlobalDisplayInfo {
   GLFWwindow* window;
   const float VIEWPORT_WIDTH = 1000;
   const float VIEWPORT_HEIGHT = 800;
} G_DISPLAY_INFO;

struct GlobalInputInfo {
   bool reset_mouse_coords = true;
   bool key_combo_pressed = false;
   bool is_mouse_left_btn_press = false;
   bool is_mouse_drag = false;
   double last_registered_mouse_coord_x = 0;
   double last_registered_mouse_coord_y = 0; 
   double mouse_btn_down_x;
   double mouse_btn_down_y;
   double currentMouseX;
   double currentMouseY;
} G_INPUT_INFO;

struct GlobalFrameInfo {
   float delta_time;
   float last_frame_time;
   int frame_counter;
   float current_fps;
   u16 frame_counter_3 = 0;
   u16 frame_counter_10 = 0;
} G_FRAME_INFO;

#include <mesh.h>

void print_vec(glm::vec3 vec, std::string prefix)
{
   std::cout << prefix << ": (" << vec.x << ", " << vec.y << ", " << vec.z << ") \n";
}

void print_vertex_array_position(Vertex* vertex, size_t length, std::string title)
{
   std::cout << title << "\n";
   for(int i = 0; i < length; i++)
   {
      glm::vec3 pos = vertex[i].position;
      std::cout << "[" << i << "] : (" << pos.x << ", " << pos.y << ", " << pos.z << ") \n";
   }
}

void print_vec_every_3rd_frame(glm::vec3 vec, std::string prefix)
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
#include <render.h>
//#include <Renderer.h>

// GLOBAL STRUCT VARIABLES (WITH CUSTOM TYPES)
GlobalEntityInfo G_ENTITY_INFO;

enum ViewMode {
   FREE_ROAM = 0,
   FIRST_PERSON = 1
};

struct GlobalSceneInfo {
   Scene* active_scene = NULL;
   Camera* camera;
   Player* player;
   ViewMode view_mode = FREE_ROAM;
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

// catalogues 
std::map<string, Mesh*> Geometry_Catalogue;
std::map<string, Shader*> Shader_Catalogue;
std::map<string, Texture> Texture_Catalogue;

#include <input.h>
#include <collision.h>
#include <scene.h>
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

// OPENGL OBJECTS
unsigned int texture, texture_specular;
// Shader quad_shader, model_shader, Text_shader, line_shader;


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
void render_text(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1,1,1));
void update_player_state(Player* player);
void adjust_player_position_and_velocity(Player* player, float distance, glm::vec3 velocity);
// void render_model(Entity ent, glm::vec3 lightPos[], glm::vec3 lightRgb[]);
// void render_scene_lights();
// unsigned int setup_object(MeshData objData);
EntityBuffer* allocate_entity_buffer(size_t size);
void update_buffers();
void handle_input_flags(int flags, Player* &player);
void check_view_mode();


int main() {

   // reads from camera position file
   float* camera_pos = load_camera_settings(PROJECT_PATH + "/camera.txt");
   std::cout << "camera " << camera_pos[0] << "," << camera_pos[1] << "," << camera_pos[2] << "\n";
   std::cout << "camera dir " << camera_pos[3] << "," << camera_pos[4] << "," << camera_pos[5] << "\n";
	Camera* new_camera = camera_create(
      vec3(camera_pos[0], camera_pos[1], camera_pos[2]), vec3(camera_pos[3], camera_pos[4], camera_pos[5]), false
   );
	G_SCENE_INFO.camera = new_camera;


	// INITIAL GLFW AND GLAD SETUPS
	setup_window(true);

	// gl enables
	glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// SHADERS
	load_text_textures("Consola.ttf", 12);
   initialize_shaders();
   create_boilerplate_geometry();

   load_scene_from_file(PROJECT_PATH + "/test.txt");

   Player* player = G_SCENE_INFO.player;

   // Allocate buffers
   EntityBuffer* entity_buffer = allocate_entity_buffer(50);
   G_BUFFERS.buffers[0] = entity_buffer;

	// MAIN LOOP
	while (!glfwWindowShouldClose(G_DISPLAY_INFO.window))
	{
      // START FRAME
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

		//	INPUT PHASE
      int input_flags = input_phase(player);
      handle_input_flags(input_flags, player);

      check_view_mode();

		//	UPDATE PHASE
		camera_update(G_SCENE_INFO.camera, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT);
      update_buffers();
      update_player_state(player);
		update_scene_objects();


		//	RENDER PHASE
		glClearColor(0.196, 0.298, 0.3607, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_scene(G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
      render_text_overlay(G_SCENE_INFO.camera, player);

      // FINISH FRAME
		glfwSwapBuffers(G_DISPLAY_INFO.window);
	}

	glfwTerminate();
	return 0;
}

void check_view_mode()
{
   if(G_SCENE_INFO.view_mode == FREE_ROAM)
   {
      // do nothing
   }
   else if(G_SCENE_INFO.view_mode == FIRST_PERSON)
   {

   }
}

void handle_input_flags(int flags, Player* &player)
{
   if(flags & KEY_PRESS_K)
   {
      load_scene_from_file(PROJECT_PATH + "/test.txt");
      player = G_SCENE_INFO.player;
   }
   if(flags & KEY_PRESS_F)
   {
      //G_SCENE_INFO.dont_render_player = true;
   }
   if(flags & KEY_PRESS_ESC)
   {
       glfwSetWindowShouldClose(G_DISPLAY_INFO.window, true);
   }

   if(G_SCENE_INFO.view_mode == FREE_ROAM)
   {
      float camera_speed = G_FRAME_INFO.delta_time * G_SCENE_INFO.camera->Acceleration;
      if(flags & KEY_PRESS_LEFT_SHIFT)
      {
         camera_speed = camera_speed * 2;
      }
      if(flags & KEY_PRESS_LEFT_CTRL)
      {
         camera_speed = camera_speed / 2;
      }
      if(flags & KEY_PRESS_W)
      {
         G_SCENE_INFO.camera->Position += camera_speed * G_SCENE_INFO.camera->Front;
      }
      if(flags & KEY_PRESS_A)
      {
         G_SCENE_INFO.camera->Position -= camera_speed * glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
      }
      if(flags & KEY_PRESS_S)
      {
         G_SCENE_INFO.camera->Position -= camera_speed * G_SCENE_INFO.camera->Front;
      }
      if(flags & KEY_PRESS_D)
      {
         G_SCENE_INFO.camera->Position += camera_speed * glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
      }
      if(flags & KEY_PRESS_Q)
      {
         G_SCENE_INFO.camera->Position -= camera_speed * G_SCENE_INFO.camera->Up;
      }
      if(flags & KEY_PRESS_O)
      {
         camera_look_at(G_SCENE_INFO.camera, glm::vec3(0.0f, 0.0f, 0.0f), true);
      }

      auto player = G_SCENE_INFO.player;
      if(player->player_state == PLAYER_STATE_STANDING)
      {
         // resets velocity
         player->entity_ptr->velocity = glm::vec3(0); 

         if (flags & KEY_PRESS_UP)
         {
            player->entity_ptr->velocity += glm::vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
         }
         if (flags & KEY_PRESS_DOWN)
         {
            player->entity_ptr->velocity -= glm::vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
         }
         if (flags & KEY_PRESS_LEFT)
         {
            glm::vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
            player->entity_ptr->velocity -= glm::vec3(onwards_vector.x, 0, onwards_vector.z);
         }
         if (flags & KEY_PRESS_RIGHT)
         {
            glm::vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
            player->entity_ptr->velocity += glm::vec3(onwards_vector.x, 0, onwards_vector.z);
         }
         // because above we sum all combos of keys pressed, here we normalize the direction and give the movement intensity
         if(glm::length2(player->entity_ptr->velocity) > 0)
         {
            float player_frame_speed = player->speed;
            if(flags & KEY_PRESS_LEFT_SHIFT)  // PLAYER DASH
               player_frame_speed *= 2;

            player->entity_ptr->velocity = player_frame_speed * glm::normalize(player->entity_ptr->velocity);
         }
         if (flags & KEY_PRESS_SPACE) 
         {
            player->player_state = PLAYER_STATE_JUMPING;
            player->entity_ptr->velocity.y = player->jump_initial_speed;
         }
      }
   }
   else if(G_SCENE_INFO.view_mode == FIRST_PERSON)
   {

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
   text_mesh->gl_data = text_gl_data;
   Geometry_Catalogue.insert({"text", text_mesh});

   // GEOMETRY

   // AABB
   vector<Vertex> aabb_vertex_vec = {
      // bottom
      Vertex{glm::vec3(0.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, -1.0f),glm::vec2(0.0f, 0.0f)},   //0
      Vertex{glm::vec3(1.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, -1.0f),glm::vec2(0.5f, 0.0f)},   //1
      Vertex{glm::vec3(1.0f, 0.0f, 1.0f),glm::vec3(0.0f, 0.0f, -1.0f),glm::vec2(0.5f, 0.5f)},   //2
      Vertex{glm::vec3(0.0f, 0.0f, 1.0f),glm::vec3(0.0f, 0.0f, -1.0f),glm::vec2(0.0f, 0.5f)},   //3
      // top
      Vertex{glm::vec3(0.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 0.0f)},    //4
      Vertex{glm::vec3(1.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.5f, 0.0f)},    //5
      Vertex{glm::vec3(1.0f, 1.0f, 1.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.5f, 0.5f)},    //6
      Vertex{glm::vec3(0.0f, 1.0f, 1.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 0.5f)}     //7
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
   aabb_mesh->vertices = aabb_vertex_vec;
   aabb_mesh->indices = aabb_vertex_indices;
   aabb_mesh->render_method = GL_TRIANGLES;
   aabb_mesh->gl_data = setup_gl_data_for_mesh(aabb_mesh);
   Geometry_Catalogue.insert({"aabb", aabb_mesh});

   // QUAD VBO
   vector<Vertex> quad_vertex_vec = {
      Vertex{glm::vec3(0.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 0.0f)},
      Vertex{glm::vec3(1.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(1.0f, 0.0f)},
      Vertex{glm::vec3(1.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(1.0f, 1.0f)},
      Vertex{glm::vec3(0.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 1.0f)}
   };
   // QUAD EBO
   vector<u32> quad_vertex_indices = { 0, 1, 2, 2, 3, 0 };


   // LINE (position is updated directly into VBO)
   /*vector<Vertex> line_vertex_vec = {
         Vertex{glm::vec3(1, 1, 0)},
         Vertex{glm::vec3(1, 1, 1)},
         Vertex{glm::vec3(0, 1, 1)},
         Vertex{glm::vec3(0, 1, 0)}
   };*/

   Mesh* quad_mesh = new Mesh();
   quad_mesh->vertices = quad_vertex_vec;
   quad_mesh->indices = quad_vertex_indices;
   quad_mesh->render_method = GL_TRIANGLES;
   quad_mesh->gl_data = setup_gl_data_for_mesh(quad_mesh);
   Geometry_Catalogue.insert({"quad", quad_mesh});

 // QUAD HORIZONTAL
   vector<Vertex> quad_horizontal_vertex_vec = {
      Vertex{glm::vec3(0.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 0.0f)},
      Vertex{glm::vec3(1.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(1.0f, 0.0f)},
      Vertex{glm::vec3(1.0f, 0.0f, 1.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(1.0f, 1.0f)},
      Vertex{glm::vec3(0.0f, 0.0f, 1.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 1.0f)}
   };

   Mesh* quad_horizontal_mesh = new Mesh();
   quad_horizontal_mesh->vertices = quad_horizontal_vertex_vec;
   quad_horizontal_mesh->indices = quad_vertex_indices;
   quad_horizontal_mesh->render_method = GL_TRIANGLES;
   quad_horizontal_mesh->gl_data = setup_gl_data_for_mesh(quad_horizontal_mesh);
   Geometry_Catalogue.insert({"quad_horizontal", quad_horizontal_mesh});


   // PLAYER CYLINDER
   Mesh* cylinder_mesh = new Mesh();
   cylinder_mesh->vertices = construct_cylinder(CYLINDER_RADIUS, CYLINDER_HALF_HEIGHT, 24);
   cylinder_mesh->render_method = GL_TRIANGLE_STRIP;
   cylinder_mesh->gl_data = setup_gl_data_for_mesh(cylinder_mesh);
   Geometry_Catalogue.insert({"player_cylinder", cylinder_mesh});
}

void update_player_state(Player* player)
{
   Entity* &player_entity = player->entity_ptr;

   // makes player move
   auto player_prior_position = player_entity->position;
   player_entity->position += player_entity->velocity * G_FRAME_INFO.delta_time;

   switch(player->player_state)
   {
      case PLAYER_STATE_FALLING:
      {
         player->entity_ptr->velocity.y -= G_FRAME_INFO.delta_time * player->fall_acceleration;

         // test collision with every object in scene entities vector
         Entity** entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
         size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();
         run_collision_checks_falling(player, entity_iterator, entity_list_size);
         break;
      }
      case PLAYER_STATE_STANDING:
      {
         // step 1: check if player is colliding with a wall
         {
            Entity** entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
            size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();
            // check for collisions with scene BUT with floor
            run_collision_checks_standing(player, entity_iterator, entity_list_size);
         }

         // step 2: check if player is still standing
         {
            auto terrain_collision = sample_terrain_height_below_player(player_entity, player->standing_entity_ptr);
            if(!terrain_collision.is_collided)
            {
               // make player "slide" towards edge and fall away from floor
               std::cout << "PLAYER FELL" << "\n";
               player_entity->velocity *= 1.3;
               player_entity->velocity.y = - 1 * player->fall_speed;
               player->player_state = PLAYER_STATE_FALLING_FROM_EDGE;
            }
         }
         break;
      }
      case PLAYER_STATE_FALLING_FROM_EDGE:
      {
         // Here it is assumed player ALREADY has a velocity vec pushing him away from the platform he is standing on
         assert(glm::length(player_entity->velocity) > 0);
         // check if still colliding with floor, if so, let player keep sliding, if not, change to FALLING
         Collision c_test = get_horizontal_overlap_player_aabb(player->standing_entity_ptr, player_entity);
         if(!c_test.is_collided)
         {
            player->player_state = PLAYER_STATE_FALLING;
            player->standing_entity_ptr = NULL;
            player_entity->velocity = glm::vec3(0, 0, 0); 
         }
         break;
      }
      case PLAYER_STATE_JUMPING:
      {
         /* remarks about the jump system:
            we set at input press time (input.h) a high velocity upward for the player
            at each frame we decrement a little bit from the y velocity component using delta frame time
            IDEALLY we would set our target jump height and let the math work itself out from there.
            For our prototype this should be fine.
         */
         //dampen player speed (vf = v0 - g*t)
         player->entity_ptr->velocity.y -= G_FRAME_INFO.delta_time * player->fall_acceleration;
         if (player->entity_ptr->velocity.y <= 0)
         {
            player->entity_ptr->velocity.y = 0;
            player->player_state = PLAYER_STATE_FALLING;
         }

         // test collision with every object in scene entities vector
         Entity** entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
         size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();
         run_collision_checks_falling(player, entity_iterator, entity_list_size);
         break;
      }
   }

   //print_vec_every_3rd_frame(player_entity->velocity, "player velocity");
   //print_vec(player_entity->position, "player position");
   //print_vec(player_entity->velocity, "player velocity");
} 

void render_text_overlay(Camera* camera, Player* player) 
{
   // render info text
   float GUI_x = 25;
   float GUI_y = G_DISPLAY_INFO.VIEWPORT_HEIGHT - 60;

   string GUI_atts[]{
      format_float_tostr(camera->Position.x, 2),                //0
      format_float_tostr(camera->Position.y,2),                 //1
      format_float_tostr(camera->Position.z,2),                 //2
      format_float_tostr(camera->Pitch,2),                      //3
      format_float_tostr(camera->Yaw,2),                        //4
      format_float_tostr(camera->Front.x,2),                    //5
      format_float_tostr(camera->Front.y,2),                    //6
      format_float_tostr(camera->Front.z,2),                    //7
      format_float_tostr(player->entity_ptr->position.x,1),    //8
      format_float_tostr(player->entity_ptr->position.y,1),    //9 
      format_float_tostr(player->entity_ptr->position.z,1)     //10
   };
 
   string camera_position = "camera:   x: " + GUI_atts[0] + " y:" + GUI_atts[1] + " z:" + GUI_atts[2];
   string camera_front    = "    dir:  x: " + GUI_atts[5] + " y:" + GUI_atts[6] + " z:" + GUI_atts[7];
   string mouse_stats     = "    pitch: " + GUI_atts[3] + " yaw: " + GUI_atts[4];
   string fps             = to_string(G_FRAME_INFO.current_fps);
   string fps_gui         = "FPS: " + fps.substr(0, fps.find('.', 0) + 2);
   string player_pos      = "player:   x: " +  GUI_atts[8] + " y: " +  GUI_atts[9] + " z: " +  GUI_atts[10];

   glm::vec3 player_state_text_color;
   std::string player_state_text;
   switch(player->player_state)
   {
      case PLAYER_STATE_STANDING:
         player_state_text_color = glm::vec3(0, 0.8, 0.1);
         player_state_text = "PLAYER STANDING";
         break;
      case PLAYER_STATE_FALLING:
         player_state_text_color = glm::vec3(0.8, 0.1, 0.1);
         player_state_text = "PLAYER FALLING";
         break;
      case PLAYER_STATE_FALLING_FROM_EDGE:
         player_state_text_color = glm::vec3(0.8, 0.1, 0.3);
         player_state_text = "PLAYER FALLING FROM EDGE";
         break;
      case PLAYER_STATE_JUMPING:
         player_state_text_color = glm::vec3(0.1, 0.3, 0.8);
         player_state_text = "PLAYER JUMPING";
         break;
   }

   float scale = 1;
   render_text(camera_position,  GUI_x, GUI_y, scale);
   render_text(camera_front,     GUI_x, GUI_y - 25, scale);
   render_text(mouse_stats,      GUI_x, GUI_y - 50, scale);
   render_text(player_pos,       GUI_x, GUI_y - 75, scale);
   render_text(fps_gui,          G_DISPLAY_INFO.VIEWPORT_WIDTH - 100, 25, scale);
   render_text(player_state_text,     GUI_x, 25, 1.4, player_state_text_color);
}



void initialize_shaders() 
{
   // text shader
	auto text_shader = create_shader_program("Text Shader", "vertex_text", "fragment_text");
   text_shader->use();
	text_shader->setMatrix4("projection", glm::ortho(0.0f, G_DISPLAY_INFO.VIEWPORT_WIDTH, 0.0f, G_DISPLAY_INFO.VIEWPORT_HEIGHT));
   Shader_Catalogue.insert({"text", text_shader});

   // general model shader
   auto model_shader = create_shader_program("Model Shader", "vertex_model", "fragment_multiple_lights");
   Shader_Catalogue.insert({"model", model_shader});

   // draw line shader
	auto line_shader = create_shader_program("Line Shader", "vertex_debug_line", "fragment_debug_line");
   Shader_Catalogue.insert({"line", line_shader});
}


std::string format_float_tostr(float num, int precision) 
{
	string temp = std::to_string(num);
	return temp.substr(0, temp.find(".") + 3);
}


void render_text(std::string text, float x, float y, float scale, glm::vec3 color) 
{
   auto find1 = Shader_Catalogue.find("text");
   auto text_shader = find1->second;
	text_shader->use();
	text_shader->setFloat3("textColor", color.x, color.y, color.z);

   auto find2 = Geometry_Catalogue.find("text");
   Mesh* text_geometry = find2->second;
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(text_geometry->gl_data.VAO);

	std::string::iterator c;
	for (c = text.begin(); c != text.end(); c++) 
   {
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
         { xpos, ypos + h, 0.0, 0.0 },
         { xpos, ypos, 0.0, 1.0 },
         { xpos + w, ypos, 1.0, 1.0 },
         { xpos, ypos + h, 0.0, 0.0 },
         { xpos + w, ypos, 1.0, 1.0 },
         { xpos + w, ypos + h, 1.0, 0.0 }
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, text_geometry->gl_data.VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
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
		mat4 model = translate(mat4identity, entity->position);
		model = rotate(model, radians(entity->rotation.x), vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, radians(entity->rotation.z), vec3(0.0f, 0.0f, 1.0f));
		model = scale(model, entity->scale);
		entity->matModel = model;

      entity_iterator++;
	}
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
