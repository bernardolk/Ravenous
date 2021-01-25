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


typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long int u64;
const float PI = 3.141592;

const std::string textures_path = "w:/assets/textures/";
const std::string models_path = "w:/assets/models/";
const std::string FONTS_PATH = "w:/assets/fonts/";

const float VIEWPORT_WIDTH = 1000;
const float VIEWPORT_HEIGHT = 800;

const glm::mat4 mat4identity(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);


#include <text.h>
#include <Shader.h>
#include <Mesh.h>
#include <Model.h>
#include <Camera.h>
#include <Entities.h>
#include <Renderer.h>
#include <parser.h>


#define glCheckError() glCheckError_(__FILE__, __LINE__) 

using namespace glm;

// SHADER SETTINGS
float global_shininess = 32.0f;

// OPENGL OBJECTS
unsigned int texture, texture_specular;
Shader quad_shader, model_shader, Text_shader;

// FUNCTION PROTOTYPES
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void onMouseMove(GLFWwindow* window, double xpos, double ypos);
// void render_model(Entity ent, glm::vec3 lightPos[], glm::vec3 lightRgb[]);
void setup_window(bool debug);
void onMouseBtn(GLFWwindow* window, int button, int action, int mods);
void render_ray();
void render_scene();
void update_scene_objects();
//void render_scene_lights();
void initialize_shaders();
void editor_render_gui(Camera& camera);
//unsigned int setup_object(MeshData objData);
Entity make_platform(float y, float x, float z, float length, float width, Model model, Shader shader);
GLenum glCheckError_(const char* file, int line);
std::string format_float_tostr(float num, int precision);
void render_text(std::string text, float x, float y, float scale, glm::vec3 color);


// Variables
GlobalEntityInfo G_ENTITY_INFO;
GLFWwindow* window;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int frameCounter = 0;
float current_fps;
bool editor_mode = true;

Scene* active_scene;
Camera active_camera;
GLuint Text_VAO, Text_VBO;

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


int main() {

   // reads from camera position file
   float* camera_pos = load_camera_settings("w:/camera.txt");
   std::cout << "camera " << camera_pos[0] << "," << camera_pos[1] << "," << camera_pos[2] << "\n";
   std::cout << "camera dir " << camera_pos[3] << "," << camera_pos[4] << "," << camera_pos[5] << "\n";
	u16 camera_id = 
            camera_create(vec3(camera_pos[0], camera_pos[1], camera_pos[2]), vec3(camera_pos[3], camera_pos[4], camera_pos[5]), false);
	active_camera = cameraList[camera_id];


	// INITIAL GLFW AND GLAD SETUPS
	setup_window(true);

	// gl enables
	glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// shaders
	model_shader = create_shader_program("Model Shader", "vertex_model", "fragment_multiple_lights");
	Shader obj_shader = create_shader_program("Obj Shader", "vertex_color_cube", "fragment_multiple_lights");
	Shader light_shader = create_shader_program("Light Props Shader", "vertex_color_cube", "fragment_light");
	//quad_shader = create_shader_program("Billboard Shader", "quad_vertex", "textured_quad_fragment");
	quad_shader = create_shader_program("Debug", "quad_vertex", "fragment_multiple_lights");

	// Text shaders (GUI)
	load_text_textures("Consola.ttf", 12);
   initialize_shaders();

	// CREATE SCENE 
   Scene demo_scene;
   demo_scene.id = 1;

   Mesh quad_mesh = Mesh(quad_vertex_vec, quad_vertex_indices);
   Model quad_model(quad_mesh);
   //Entity plat = make_platform(1.0, 1.0, 1.0, 5.0, 3.0, quad_model, quad_shader);
   //demo_scene.entities.push_back(plat);

   Entity quad_wall{
      G_ENTITY_INFO.entity_counter,
      ++G_ENTITY_INFO.entity_counter,
      &quad_model,
      &model_shader,
      vec3(0,0,0),
      vec3(90, 0, 90),
      vec3(1.0f,1.0f,1.0f)
   };
   demo_scene.entities.push_back(quad_wall);


   // LIGHTSOURCES
   PointLight l1;
   l1.id = 1;
   l1.position = vec3(-3, 1.5, -1.5);
   l1.diffuse = vec3(1.0, 1.0, 1.0);
   l1.intensity_linear = 0.05f;
   l1.intensity_quadratic = 0.001f;
   demo_scene.pointLights.push_back(l1);

   active_scene = &demo_scene;
   
	// MAIN LOOP
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		current_fps = 1.0f / deltaTime;

		//	INPUT PHASE
		glfwPollEvents();
		processInput(window);

		//	UPDATE PHASE
		camera_update(active_camera, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
		update_scene_objects();

		//	RENDER PHASE
		glClearColor(0.196, 0.298, 0.3607, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render_scene();

      editor_render_gui(active_camera);

		glfwSwapBuffers(window);
	}

	//editor_terminate();
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


void editor_render_gui(Camera& camera) 
{
   // render GUI text
   // text render
   float GUI_x = 25;
   float GUI_y = VIEWPORT_HEIGHT - 60;

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
   string fps = to_string(current_fps);
   string fps_gui = "FPS: " + fps.substr(0, fps.find('.', 0) + 2);


   float scale = 1;
   render_text(camera_position, GUI_x, GUI_y, scale, glm::vec3(1.0f, 1.0f, 1.0f));
   render_text(camera_front, GUI_x, GUI_y - 25, scale, glm::vec3(1.0f, 1.0f, 1.0f));
   render_text(mouse_stats, GUI_x, GUI_y - 50, scale, glm::vec3(1.0f, 1.0f, 1.0f));
   render_text(fps_gui, VIEWPORT_HEIGHT - 100, 25, scale, glm::vec3(1.0f, 1.0f, 1.0f));
}



void initialize_shaders() 
{
   // text shader
	Text_shader = create_shader_program("Text Shader", "vertex_text", "fragment_text");
   Text_shader.use();
	Text_shader.setMatrix4("projection", glm::ortho(0.0f, VIEWPORT_WIDTH, 0.0f, VIEWPORT_HEIGHT));

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
	auto entity = active_scene->entities.begin();
	auto end = active_scene->entities.end();
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
	window = glfwCreateWindow(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, "Ravenous", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Setups openGL viewport
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, onMouseMove);
	glfwSetScrollCallback(window, onMouseScroll);
	glfwSetMouseButtonCallback(window, onMouseBtn);

	if (debug) {
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	}

}


void render_scene() 
{
	auto entity_ptr = active_scene->entities.begin();
	for (entity_ptr; entity_ptr != active_scene->entities.end(); entity_ptr++) 
   {
		entity_ptr->shader->use();
		auto point_light_ptr = active_scene->pointLights.begin();
		int point_light_count = 0;
		for (point_light_ptr; point_light_ptr != active_scene->pointLights.end(); point_light_ptr++)
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
		entity_ptr->shader->setInt("num_point_lights"		,point_light_count);
		entity_ptr->shader->setInt("num_directional_light"	, 0);
		entity_ptr->shader->setInt("num_spot_lights"		, 0);
		entity_ptr->shader->setMatrix4("view"				, active_camera.View4x4);
		entity_ptr->shader->setMatrix4("projection"			, active_camera.Projection4x4);
		entity_ptr->shader->setFloat("shininess"			, global_shininess);
		entity_ptr->shader->setFloat3("viewPos"				, active_camera.Position);
		//mat4 model_matrix = scale(mat4identity				, vec3(0.01,0.01,0.01));
		entity_ptr->shader->setMatrix4("model"				, entity_ptr->matModel);
		entity_ptr->model3d->Draw(*entity_ptr->shader);
	}
}

void processInput(GLFWwindow* window)
{
	//Todo: get a real input toggling system in place
	// something that allows you to wait for release to get in the if again
	float cameraSpeed = deltaTime * active_camera.Acceleration;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) {

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			cameraSpeed = deltaTime * active_camera.Acceleration * 2;
		}
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			active_camera.Position += cameraSpeed * active_camera.Front;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			active_camera.Position -= cameraSpeed * active_camera.Front;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			active_camera.Position -= cameraSpeed * glm::normalize(glm::cross(active_camera.Front, active_camera.Up));
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			active_camera.Position += cameraSpeed * glm::normalize(glm::cross(active_camera.Front, active_camera.Up));
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			active_camera.Position -= cameraSpeed * active_camera.Up;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			active_camera.Position += cameraSpeed * active_camera.Up;
		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
			camera_look_at(active_camera, glm::vec3(0.0f, 0.0f, 0.0f), true);
			G_INPUT_INFO.reset_mouse_coords = true;
		}
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
			global_shininess -= 10 * deltaTime;
			if (global_shininess < 1)
				global_shininess = 1;
		}
		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
			global_shininess += 10 * deltaTime;
      if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
			save_camera_settings_to_file("w:/camera.txt", active_camera.Position, active_camera.Front);
		}
	}

	// This solution will only work while i have only one key combo implemented (i guess)
	if (G_INPUT_INFO.key_combo_pressed) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
			G_INPUT_INFO.key_combo_pressed = false;
	}

}

void onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
   if (G_INPUT_INFO.is_mouse_drag) {
      // 'teleports' stored coordinates to current mouse coordinates
      if (G_INPUT_INFO.reset_mouse_coords) {
         G_INPUT_INFO.last_registered_mouse_coord_x = xpos;
         G_INPUT_INFO.last_registered_mouse_coord_y = ypos;
         G_INPUT_INFO.reset_mouse_coords = false;
      }

      // calculates offsets
      float xoffset = xpos - G_INPUT_INFO.last_registered_mouse_coord_x;
      float yoffset = G_INPUT_INFO.last_registered_mouse_coord_y - ypos;
      G_INPUT_INFO.last_registered_mouse_coord_x = xpos;
      G_INPUT_INFO.last_registered_mouse_coord_y = ypos;

      xoffset *= active_camera.Sensitivity;
      yoffset *= active_camera.Sensitivity;

      camera_change_direction(active_camera, xoffset, yoffset);

      // Unallows camera to perform a flip
      if (active_camera.Pitch > 89.0f)
         active_camera.Pitch = 89.0f;
      if (active_camera.Pitch < -89.0f)
         active_camera.Pitch = -89.0f;

      // Make sure we don't overflow floats when camera is spinning indefinetely
      if (active_camera.Yaw > 360.0f)
         active_camera.Yaw = active_camera.Yaw - 360.0f;
      if (active_camera.Yaw < -360.0f)
         active_camera.Yaw = active_camera.Yaw + 360.0f;
   }

   G_INPUT_INFO.currentMouseX = xpos;
	G_INPUT_INFO.currentMouseY = ypos;

	// mouse dragging controls
	if (G_INPUT_INFO.is_mouse_left_btn_press
		&& (abs(G_INPUT_INFO.mouse_btn_down_x - G_INPUT_INFO.currentMouseX) > 2
			|| abs(G_INPUT_INFO.mouse_btn_down_y - G_INPUT_INFO.currentMouseY) > 2)) {
		G_INPUT_INFO.is_mouse_drag = true;
	}
}

void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) 
{
		active_camera.Position += (float)(3 * yoffset) * active_camera.Front;
}

void onMouseBtn(GLFWwindow* window, int button, int action, int mods) 
{
   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
   {
      G_INPUT_INFO.is_mouse_left_btn_press = true;
      G_INPUT_INFO.reset_mouse_coords = true;
      G_INPUT_INFO.mouse_btn_down_x = G_INPUT_INFO.currentMouseX;
      G_INPUT_INFO.mouse_btn_down_y = G_INPUT_INFO.currentMouseY;
   }
   else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) 
   {
      cout << "left_btn_release" << endl;
      G_INPUT_INFO.is_mouse_left_btn_press = false;
      cout << G_INPUT_INFO.is_mouse_drag << endl;
      G_INPUT_INFO.is_mouse_drag = false;
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
