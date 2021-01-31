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

const std::string textures_path = "w:/assets/textures/";
const std::string models_path = "w:/assets/models/";
const std::string FONTS_PATH = "w:/assets/fonts/";

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

struct GlobalSceneInfo {
   Scene* active_scene;
   Camera camera;
} G_SCENE_INFO;

#include <input.h>
#include <collision.h>
#include <scene.h>
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

// OPENGL OBJECTS
unsigned int texture, texture_specular;
Shader quad_shader, model_shader, Text_shader, line_shader;

// catalogues 
std::map<string, Model*> Model_Catalogue;
std::map<string, Shader> Shader_Catalogue;
std::map<string, Texture> Texture_Catalogue;


using namespace glm;

// FUNCTION PROTOTYPES
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void setup_window(bool debug);
void render_ray();
void update_scene_objects();
void initialize_shaders();
void initialize_models();
void render_text_overlay(Camera& camera);
GLenum glCheckError_(const char* file, int line);
std::string format_float_tostr(float num, int precision);
void render_text(std::string text, float x, float y, float scale, glm::vec3 color);
void update_player_state(Player* player);
void adjust_player_position_and_velocity(Player* player, float distance, glm::vec3 velocity);
// void render_model(Entity ent, glm::vec3 lightPos[], glm::vec3 lightRgb[]);
// void render_scene_lights();
// unsigned int setup_object(MeshData objData);

int main() {

   // reads from camera position file
   float* camera_pos = load_camera_settings("w:/camera.txt");
   std::cout << "camera " << camera_pos[0] << "," << camera_pos[1] << "," << camera_pos[2] << "\n";
   std::cout << "camera dir " << camera_pos[3] << "," << camera_pos[4] << "," << camera_pos[5] << "\n";
	u16 camera_id = 
            camera_create(vec3(camera_pos[0], camera_pos[1], camera_pos[2]), vec3(camera_pos[3], camera_pos[4], camera_pos[5]), false);
	G_SCENE_INFO.camera = cameraList[camera_id];


	// INITIAL GLFW AND GLAD SETUPS
	setup_window(true);

	// gl enables
	glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// SHADERS
	load_text_textures("Consola.ttf", 12);
   initialize_shaders();
   initialize_models();

   // creates the scene (objects and player)
	#include<scene_description.h>

   load_scene_entities_from_file("w:/test.txt");
   
	// MAIN LOOP
	while (!glfwWindowShouldClose(G_DISPLAY_INFO.window))
	{
      // START FRAME
		float currentFrame = glfwGetTime();
		G_FRAME_INFO.delta_time = currentFrame - G_FRAME_INFO.last_frame_time;
      if(G_FRAME_INFO.delta_time > 0.02)
      {
         // @BUG: Player will stutter if we set delta_time here below. Why?
         // G_FRAME_INFO.delta_time = 0.2;
         std::cout << "delta time exceeded.\n";
      } 
		G_FRAME_INFO.last_frame_time = currentFrame;
		G_FRAME_INFO.current_fps = 1.0f / G_FRAME_INFO.delta_time;
      G_FRAME_INFO.frame_counter_3 = ++G_FRAME_INFO.frame_counter_3 % 3;
      G_FRAME_INFO.frame_counter_10 = ++G_FRAME_INFO.frame_counter_10 % 10;   

		//	INPUT PHASE
      input_phase(&player);

		//	UPDATE PHASE
		camera_update(G_SCENE_INFO.camera, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT);
      update_player_state(&player);
		update_scene_objects();


		//	RENDER PHASE
		glClearColor(0.196, 0.298, 0.3607, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_scene(G_SCENE_INFO.active_scene, &G_SCENE_INFO.camera);
      render_text_overlay(G_SCENE_INFO.camera);

      // FINISH FRAME
		glfwSwapBuffers(G_DISPLAY_INFO.window);
	}

	glfwTerminate();
	return 0;
}

void initialize_models()
{
   //TEXT
   GLData text_gl_data;
	glGenVertexArrays(1, &text_gl_data.VAO);
	glGenBuffers(1, &text_gl_data.VBO);
	glBindVertexArray(text_gl_data.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, text_gl_data.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

   Model* text_model = (Model*) malloc(sizeof(Model));
   text_model->gl_data = text_gl_data;
   Model_Catalogue.insert({"text", text_model});

   // GEOMETRY
      // QUAD
      // VBO
      vector<Vertex> quad_vertex_vec = {
         Vertex{glm::vec3(0.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 0.0f)},
         Vertex{glm::vec3(1.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(1.0f, 0.0f)},
         Vertex{glm::vec3(1.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(1.0f, 1.0f)},
         Vertex{glm::vec3(0.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 1.0f)}
      };
      // EBO
      vector<u32> quad_vertex_indices = { 0, 1, 2, 2, 3, 0 };

      // LINE (position is updated directly into VBO)
      vector<Vertex> line_vertex_vec = {
            Vertex{glm::vec3(1, 1, 0)},
            Vertex{glm::vec3(1, 1, 1)},
            Vertex{glm::vec3(0, 1, 1)},
            Vertex{glm::vec3(0, 1, 0)}
      };

   // TEXTURES
      unsigned int brick_texture = load_texture_from_file("brickwall.jpg", "w:/assets/textures");
      unsigned int brick_normal_texture = load_texture_from_file("brickwall_normal.jpg", "w:/assets/textures");
      unsigned int green_tex = load_texture_from_file("green.jpg", "w:/assets/textures");

      Texture quad_wall_texture{
         brick_texture,
         "texture_diffuse",
         "whatever"
      };
      Texture quad_wall_normal_texture{
         brick_normal_texture,
         "texture_normal",
         "whatever"
      };
      Texture green_texture{
         green_tex,
         "texture_diffuse",
         "whatever"
      };

      // DEFAULT TEX VEC
      vector<Texture> texture_vec;
      texture_vec.push_back(quad_wall_texture);
      texture_vec.push_back(quad_wall_normal_texture);

      // TEX VEC WITH GREEN TEX
      vector<Texture> plat_texture_vec;
      plat_texture_vec.push_back(green_texture);

   // QUAD MESH AND MODELS
      Mesh quad_mesh;
      quad_mesh.vertices = quad_vertex_vec;
      quad_mesh.indices = quad_vertex_indices;
      quad_mesh.render_method = GL_TRIANGLES;

      Model* quad_model = new Model();
      quad_model->mesh = quad_mesh;
      quad_model->textures = texture_vec;
      quad_model->gl_data = setup_gl_data_for_mesh(&quad_mesh);
      Model_Catalogue.insert({"quad", quad_model});

   // GREEN TEX MODEL
      Model* green_model = new Model();
      green_model->mesh = quad_mesh;
      green_model->textures = plat_texture_vec;
      green_model->gl_data = quad_model->gl_data;
      Model_Catalogue.insert({"green quad", green_model});

}

void update_player_state(Player* player)
{
   Entity* &player_entity = player->entity_ptr;

   // @bug: when moving towards an edge, player gets roundabouted (collision system gets crazy)

   // makes player move
   auto player_prior_position = player_entity->position;
   player_entity->position += player_entity->velocity * G_FRAME_INFO.delta_time;

   switch(player->player_state)
   {
      case PLAYER_STATE_FALLING:
      {
         // test collision with every object in scene entities vector
         Entity** entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
         size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();
         CollisionData cd = check_player_collision_with_scene(player_entity, entity_iterator, entity_list_size);

         // if collided
         if(cd.collided_entity_ptr != NULL)
         {
            // move player to collision point, stop player and set him to standing
            auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player_entity->collision_geometry_ptr;
            player_entity->position.y += player_collision_geometry->half_length - cd.distance_from_position; 
            player_entity->velocity = glm::vec3(0,0,0);
            player->player_state = PLAYER_STATE_STANDING;
            player->standing_entity_ptr = cd.collided_entity_ptr;
         }
         break;
      }
      case PLAYER_STATE_STANDING:
      {
         // step 1: check if player is colliding with a wall
         Entity** entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
         size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();
         // check for collisions with scene BUT with floor
         CollisionData cd = check_player_collision_with_scene(player_entity, entity_iterator, entity_list_size,
                                                                        player->standing_entity_ptr->name);

         // if collided with something else then the floor player is currently standing on
         if(cd.collided_entity_ptr != NULL)
         {
            // move player back to where he was last frame 
            auto player_v = player_entity->velocity;
            auto edge = get_nearest_edge(player_entity, cd.collided_entity_ptr);
            // since we know the edges are axis-aligned...
            if(edge.x == 0)
            {
               player_entity->velocity.x = 0;
               int sign = player_entity->velocity.z > 0 ? 1 : -1;
               player_entity->velocity.z = player->speed * sign;
            }
            else if(edge.z == 0)
            {
               player_entity->velocity.z = 0;
               int sign = player_entity->velocity.x > 0 ? 1 : -1;
               player_entity->velocity.x = player->speed * sign;
            }
            else{
               assert(false);
            }
            //print_vec(edge, "Edge vector");

            //player_entity->velocity = glm::vec3(0,0,0);
            player_entity->position = player_prior_position;
            player_entity->position +=  player_entity->velocity * G_FRAME_INFO.delta_time;
            break;
         }

         // step 2: check if player is still standing
         auto terrain_collision = sample_terrain_height_below_player(player_entity, player->standing_entity_ptr);
         if(!terrain_collision.collision)
         {
            std::cout << "PLAYER FELL" << "\n";
            player_entity->velocity *= 1.3;
            player_entity->velocity.y = - 1 * player->fall_speed;
            player->player_state = PLAYER_STATE_FALLING_FROM_EDGE;
         }
         break;
      }
      case PLAYER_STATE_FALLING_FROM_EDGE:
      {
         assert(glm::length(player_entity->velocity) > 0);
         bool collision = check_2D_collision_circle_and_aligned_square(player_entity, player->standing_entity_ptr);
         if(!collision)
         {
            player->player_state = PLAYER_STATE_FALLING;
            player->standing_entity_ptr = NULL;
            player_entity->velocity = glm::vec3(0, -1 * player->fall_speed, 0); 
         }
         break;
      }
   }

   print_vec_every_3rd_frame(player_entity->velocity, "player velocity");
   //print_vec(player_entity->position, "player position");
   //print_vec(player_entity->velocity, "player velocity");
   
} 

void render_text_overlay(Camera& camera) 
{
   // render info text
   float GUI_x = 25;
   float GUI_y = G_DISPLAY_INFO.VIEWPORT_HEIGHT - 60;

   string GUI_atts[]{
      format_float_tostr(camera.Position.x, 2),
      format_float_tostr(camera.Position.y,2),
      format_float_tostr(camera.Position.z,2),
      format_float_tostr(camera.Pitch,2),
      format_float_tostr(camera.Yaw,2),
      format_float_tostr(camera.Front.x,2),
      format_float_tostr(camera.Front.y,2),
      format_float_tostr(camera.Front.z,2)
   };

   string camera_position = "pos :: x: " + GUI_atts[0] + " y:" + GUI_atts[1] + " z:" + GUI_atts[2];
   string camera_front = "dir :: x: " + GUI_atts[5] + " y:" + GUI_atts[6] + " z:" + GUI_atts[7];
   string mouse_stats = "pitch: " + GUI_atts[3] + " yaw: " + GUI_atts[4];
   string fps = to_string(G_FRAME_INFO.current_fps);
   string fps_gui = "FPS: " + fps.substr(0, fps.find('.', 0) + 2);


   float scale = 1;
   render_text(camera_position, GUI_x, GUI_y, scale, glm::vec3(1.0f, 1.0f, 1.0f));
   render_text(camera_front, GUI_x, GUI_y - 25, scale, glm::vec3(1.0f, 1.0f, 1.0f));
   render_text(mouse_stats, GUI_x, GUI_y - 50, scale, glm::vec3(1.0f, 1.0f, 1.0f));
   render_text(fps_gui, G_DISPLAY_INFO.VIEWPORT_HEIGHT - 100, 25, scale, glm::vec3(1.0f, 1.0f, 1.0f));
}



void initialize_shaders() 
{
   // text shader
	Shader text_shader = create_shader_program("Text Shader", "vertex_text", "fragment_text");
   text_shader.use();
	text_shader.setMatrix4("projection", glm::ortho(0.0f, G_DISPLAY_INFO.VIEWPORT_WIDTH, 0.0f, G_DISPLAY_INFO.VIEWPORT_HEIGHT));
   Shader_Catalogue.insert({"text", text_shader});

   // general model shader
   model_shader = create_shader_program("Model Shader", "vertex_model", "fragment_multiple_lights");
   Shader_Catalogue.insert({"model", model_shader});

   // draw line shader
	line_shader = create_shader_program("Line Shader", "vertex_debug_line", "fragment_debug_line");
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
   Shader text_shader = find1->second;
	text_shader.use();
	text_shader.setFloat3("textColor", color.x, color.y, color.z);

   auto find2 = Model_Catalogue.find("text");
   Model* text_model = find2->second;
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(text_model->gl_data.VAO);

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
		glBindBuffer(GL_ARRAY_BUFFER, text_model->gl_data.VBO);
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
