// DEPENDENCY INCLUDES
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/vector_float2.hpp> // vec2
#include <glm/ext/vector_float3.hpp> // vec3
#include <glm/ext/matrix_float4x4.hpp> // mat4x4
#include <glm/ext/matrix_transform.hpp> // translate, rotate, scale, identity
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>

// DEFINES
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long int u64;
const float PI = 3.141592;

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
} G_FRAME_INFO;


// SOURCE INCLUDES
#include <text.h>
#include <shader.h>
#include <mesh.h>
#include <model.h>
#include <camera.h>
#include <entities.h>
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


#include <parser.h>
#include <input.h>
#define glCheckError() glCheckError_(__FILE__, __LINE__) 


// SHADER SETTINGS
float global_shininess = 32.0f;

// OPENGL OBJECTS
unsigned int texture, texture_specular;
Shader quad_shader, model_shader, Text_shader;


using namespace glm;

// FUNCTION PROTOTYPES
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void setup_window(bool debug);
void render_ray();
void render_scene();
void update_scene_objects();
void initialize_shaders();
void render_text_overlay(Camera& camera);
Entity make_platform(float y, float x, float z, float length, float width, Model model, Shader shader);
GLenum glCheckError_(const char* file, int line);
std::string format_float_tostr(float num, int precision);
void render_text(std::string text, float x, float y, float scale, glm::vec3 color);
// void render_model(Entity ent, glm::vec3 lightPos[], glm::vec3 lightRgb[]);
// void render_scene_lights();
// unsigned int setup_object(MeshData objData);





// text vao and vbo
GLuint Text_VAO, Text_VBO;

//std::map<std::string, Shader> Shader_catalog;


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

	// shaders
	model_shader = create_shader_program("Model Shader", "vertex_model", "fragment_multiple_lights");
	//Shader obj_shader = create_shader_program("Obj Shader", "vertex_color_cube", "fragment_multiple_lights");
	//Shader light_shader = create_shader_program("Light Props Shader", "vertex_color_cube", "fragment_light");
	//quad_shader = create_shader_program("Billboard Shader", "quad_vertex", "textured_quad_fragment");
	//quad_shader = create_shader_program("Debug", "quad_vertex", "fragment_multiple_lights");

	// Text shaders (GUI)
	load_text_textures("Consola.ttf", 12);
   initialize_shaders();

	// CREATE SCENE 
   Scene demo_scene;

   // ENTITY SETUP
   unsigned int brick_texture = load_texture_from_file("brickwall.jpg", "w:/assets/textures");
   unsigned int brick_normal_texture = load_texture_from_file("brickwall_normal.jpg", "w:/assets/textures");
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
   vector<Texture> texture_vec;
   texture_vec.push_back(quad_wall_texture);
   texture_vec.push_back(quad_wall_normal_texture);

   Mesh quad_mesh;
   quad_mesh.vertices = quad_vertex_vec;
   quad_mesh.indices = quad_vertex_indices;
   quad_mesh.render_method = GL_TRIANGLES;

   Model quad_model;
   quad_model.mesh = quad_mesh;
   quad_model.textures = texture_vec;
   quad_model.gl_data = setup_gl_data_for_mesh(&quad_mesh);

   Entity platform{
      G_ENTITY_INFO.entity_counter,
      ++G_ENTITY_INFO.entity_counter,
      &quad_model,
      &model_shader,
      vec3(0.5,0,0.5),
      vec3(90, 0, 90),
      vec3(1.0f,1.0f,1.0f)
   };
   demo_scene.entities.push_back(platform);


   // CYLINDER
   unsigned int pink_texture = load_texture_from_file("pink.jpg", "w:/assets/textures");
   Texture cylinder_texture{
      pink_texture,
      "texture_diffuse",
      "whatever"
   };

   Mesh cylinder_mesh;
   cylinder_mesh.vertices = construct_cylinder(0.15f, 0.35f, 24);
   cylinder_mesh.render_method = GL_TRIANGLE_STRIP;

   Model cylinder_model;
   cylinder_model.mesh = cylinder_mesh;
   cylinder_model.textures = std::vector<Texture>{cylinder_texture};
   cylinder_model.gl_data = setup_gl_data_for_mesh(&cylinder_mesh);

   Entity cylinder{
      G_ENTITY_INFO.entity_counter,
      ++G_ENTITY_INFO.entity_counter,
      &cylinder_model,
      &model_shader,
      vec3(0,1,1)
   };
   demo_scene.entities.push_back(cylinder);



   // lightsource
   PointLight l1;
   l1.id = 1;
   l1.position = vec3(0.5, 2.5, 0.5);
   l1.diffuse = vec3(1.0, 1.0, 1.0);
   l1.ambient = vec3(0.6,0.6,0.6);
   l1.intensity_linear = 0.4f;
   l1.intensity_quadratic = 0.04f;
   demo_scene.pointLights.push_back(l1);

   G_SCENE_INFO.active_scene = &demo_scene;
   
	// MAIN LOOP
	while (!glfwWindowShouldClose(G_DISPLAY_INFO.window))
	{
      // START FRAME
		float currentFrame = glfwGetTime();
		G_FRAME_INFO.delta_time = currentFrame - G_FRAME_INFO.last_frame_time;
		G_FRAME_INFO.last_frame_time = currentFrame;
		G_FRAME_INFO.current_fps = 1.0f / G_FRAME_INFO.delta_time;

		//	INPUT PHASE
      input_phase();

		//	UPDATE PHASE
		camera_update(G_SCENE_INFO.camera, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT);
		update_scene_objects();

		//	RENDER PHASE
		glClearColor(0.196, 0.298, 0.3607, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_scene();
      render_text_overlay(G_SCENE_INFO.camera);

      // FINISH FRAME
		glfwSwapBuffers(G_DISPLAY_INFO.window);
	}

	glfwTerminate();
	return 0;
}

// this most likely should allocate memory for the platform and return a pointer to the thing
// goddamn i dont know anything yet...
Entity make_platform(float y, float x, float z, float length, float width, Model model, Shader shader) 
{
   Entity platform{
         G_ENTITY_INFO.entity_counter,
         ++G_ENTITY_INFO.entity_counter,
         &model,
         &shader,
         vec3(x,y,z),
         vec3(90, 0, 90),
         vec3(length, 2.0f, width)
      };
   return platform;
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
	Text_shader = create_shader_program("Text Shader", "vertex_text", "fragment_text");
   Text_shader.use();
	Text_shader.setMatrix4("projection", glm::ortho(0.0f, G_DISPLAY_INFO.VIEWPORT_WIDTH, 0.0f, G_DISPLAY_INFO.VIEWPORT_HEIGHT));

	//generate text buffers
	glGenVertexArrays(1, &Text_VAO);
	glGenBuffers(1, &Text_VBO);
	glBindVertexArray(Text_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, Text_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

std::string format_float_tostr(float num, int precision) 
{
	string temp = std::to_string(num);
	return temp.substr(0, temp.find(".") + 3);
}

void render_text(std::string text, float x, float y, float scale, glm::vec3 color) 
{
	Text_shader.use();
	Text_shader.setFloat3("textColor", color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(Text_VAO);

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

		//std::cout << "xpos: " << xpos << ", ypos:" << ypos << ", h: " << h << ", w: " << w << std::endl;
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, Text_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
}

// unsigned int setup_object(MeshData objData){
// 	unsigned int objVAO, objVBO, objEBO;
// 	glGenVertexArrays(1, &objVAO);
// 	glGenBuffers(1, &objVBO);
// 	glGenBuffers(1, &objEBO);
// 	glBindVertexArray(objVAO);
// 	glBindBuffer(GL_ARRAY_BUFFER, objVBO);
// 	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * objData.vertexes.size() , &objData.vertexes[0], GL_STATIC_DRAW);	
// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objEBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, objData.indices.size() * sizeof(unsigned int),
// 		     &objData.indices[0], GL_STATIC_DRAW);
// 	// vertex Positions
// 	glEnableVertexAttribArray(0);
// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
// 	glBindVertexArray(0);

// 	return objVAO;
// }


inline void update_scene_objects() 
{
	auto entity = G_SCENE_INFO.active_scene->entities.begin();
	auto end = G_SCENE_INFO.active_scene->entities.end();
	for (entity; entity < end; entity++) 
   {
		// Updates model matrix;	
		mat4 model = translate(mat4identity, entity->position);
		model = rotate(model, radians(entity->rotation.x), vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, radians(entity->rotation.z), vec3(0.0f, 0.0f, 1.0f));
		model = scale(model, entity->scale);
		entity->matModel = model;
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


void render_scene() 
{
	Entity *entity_ptr = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
		entity_ptr->shader->use();
		auto point_light_ptr = G_SCENE_INFO.active_scene->pointLights.begin();
		int point_light_count = 0;
		for (point_light_ptr; point_light_ptr != G_SCENE_INFO.active_scene->pointLights.end(); point_light_ptr++)
      {
			PointLight point_light = *point_light_ptr;
			string uniform_name = "pointLights[" + to_string(point_light_count) + "]";
			entity_ptr->shader->setFloat3(uniform_name + ".position", point_light.position);
			entity_ptr->shader->setFloat3(uniform_name + ".diffuse", point_light.diffuse);
			entity_ptr->shader->setFloat3(uniform_name + ".specular", point_light.specular);
			entity_ptr->shader->setFloat3(uniform_name + ".ambient", point_light.ambient);
			entity_ptr->shader->setFloat(uniform_name + ".constant", point_light.intensity_constant);
			entity_ptr->shader->setFloat(uniform_name + ".linear", point_light.intensity_linear);
			entity_ptr->shader->setFloat(uniform_name + ".quadratic", point_light.intensity_quadratic);
			point_light_count++;
		}
		entity_ptr->shader->setInt    ("num_point_lights", point_light_count);
		entity_ptr->shader->setInt    ("num_directional_light", 0);
		entity_ptr->shader->setInt    ("num_spot_lights", 0);
		entity_ptr->shader->setMatrix4("view", G_SCENE_INFO.camera.View4x4);
		entity_ptr->shader->setMatrix4("projection", G_SCENE_INFO.camera.Projection4x4);
		entity_ptr->shader->setFloat  ("shininess", global_shininess);
		entity_ptr->shader->setFloat3 ("viewPos", G_SCENE_INFO.camera.Position);
		//mat4 model_matrix = scale   (mat4identity, vec3(0.01,0.01,0.01));
		entity_ptr->shader->setMatrix4("model", entity_ptr->matModel);
		render_entity(entity_ptr);

      entity_ptr++;
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
