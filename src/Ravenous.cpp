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

#include <Shader.h>
#include <Mesh.h>
#include <Model.h>
#include <Camera.h>
#include <Entities.h>
#include <Renderer.h>

#define glCheckError() glCheckError_(__FILE__, __LINE__) 

using namespace glm;


const float PI = 3.141592;

typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long int u64;


// SHADER SETTINGS
float global_shininess = 32.0f;

// OPENGL OBJECTS
unsigned int texture, texture_specular;
Shader quad_shader, model_shader;

// METHOD PROTOTYPES
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
void render_scene_lights();
Entity make_platform(float y, float x, float z, float length, float width, Model model, Shader shader);


GLenum glCheckError_(const char* file, int line);


GLFWwindow* window;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int frameCounter = 0;
float current_fps;
bool editor_mode = true;
double currentMouseX;
double currentMouseY;
Scene* active_scene;
Camera active_camera;

const float viewportWidth = 1000;
const float viewportHeight = 800;


const string textures_path = "w:/assets/textures/";
const string models_path = "w:/assets/models/";
const string fonts_path = "w:/assets/fonts/";


#include <Editor.h>
#include <parser.h>

unsigned int setup_object(MeshData objData);


int main() {


   // reads from camera position file
   float* camera_pos = load_camera_settings("w:/camera.txt");

   // Todo: read camera.front position from file as well
   // Todo: implement function that stores current camera settings to the .txt file
   // Todo: make function be called when press the '9' key


   std::cout << "camera " << camera_pos[0] << "," << camera_pos[1] << "," << camera_pos[2] << "\n";
   std::cout << "camera dir " << camera_pos[3] << "," << camera_pos[4] << "," << camera_pos[5] << "\n";
	u16 camera_id = camera_create(vec3(camera_pos[0], camera_pos[1], camera_pos[2]), vec3(camera_pos[3], camera_pos[4], camera_pos[5]));
	active_camera = cameraList[camera_id];


	// INITIAL GLFW AND GLAD SETUPS
	setup_window(true);
	// editor_initialize(viewportWidth, viewportHeight);

	// SHADERS
	glEnable(GL_DEPTH_TEST);

	// MAIN SHADERS
	//Shader model_shader("vertex_model.shd", "fragment_model.shd");
	model_shader = create_shader_program("Model Shader", "vertex_model", "fragment_multiple_lights");
	//Shader cube_shader("vertex_main.shd", "fragment_main.shd");
	Shader obj_shader = create_shader_program("Obj Shader", "vertex_color_cube", "fragment_multiple_lights");
	Shader light_shader = create_shader_program("Light Props Shader", "vertex_color_cube", "fragment_light");
	quad_shader = create_shader_program("Billboard Shader", "quad_vertex", "textured_quad_fragment");
	//Shader pink_shader = create_shader_program("Pink", "bounding_box_vertex", "bounding_box_fragment");

	// Text shaders (GUI)
	//Shader text_shader = initialize_text_shader();
	load_text_textures("Consola.ttf", 12);

	// CREATE SCENE 
   Scene demo_scene;
   demo_scene.id = 1;


   // QUAD MODEL TESTS
   unsigned int brick_texture = load_texture_from_file("brickwall.jpg", "assets/textures");
   unsigned int brick_normal_texture = load_texture_from_file("brickwall_normal.jpg", "assets/textures");
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
   Mesh quad_mesh = Mesh(quad_vertex_vec, quad_vertex_indices, texture_vec);
   Model quad_model(quad_mesh);

   quad_model.textures_loaded = texture_vec;

   Entity quad_wall{
      entity_counter,
      ++entity_counter,
      &quad_model,
      &model_shader,
      vec3(0,0,0),
      vec3(90, 0, 90),
      vec3(1.0f,1.0f,1.0f)
   };
   demo_scene.entities.push_back(quad_wall);

   Entity quad_wall2{
      entity_counter,
      ++entity_counter,
      &quad_model,
      &model_shader,
      vec3(2,-3,2),
      vec3(90, 0, 90),
      vec3(8.0f,2.0f,8.0f)
   };
   demo_scene.entities.push_back(quad_wall2);

   //Entity platform = make_platform(-3.0f, -3.0f, 0.0f, 20, 20, quad_model, quad_shader);
   //demo_scene.entities.push_back(platform);


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
		//editor_start_frame();
		processInput(window);


		//	UPDATE PHASE
		//editor_update();
		camera_update(active_camera, viewportWidth, viewportHeight);
		update_scene_objects();


		//	RENDER PHASE
		glClearColor(0.196, 0.298, 0.3607, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render_scene();

		//if (!moveMode)
		//	render_scene_lights();

	   //editor_loop();

		//editor_end_frame();	
		glfwSwapBuffers(window);
	}

	//editor_terminate();
	glfwTerminate();
	return 0;
}

Entity make_platform(float y, float x, float z, float length, float width, Model model, Shader shader) {
   Entity platform{
         entity_counter,
         ++entity_counter,
         &model,
         &shader,
         vec3(x,y,z),
         vec3(90, 0, 90),
         vec3(length, 2.0f, width)
      };
   return platform;
}

unsigned int setup_object(MeshData objData){
	unsigned int objVAO, objVBO, objEBO;
	glGenVertexArrays(1, &objVAO);
	glGenBuffers(1, &objVBO);
	glGenBuffers(1, &objEBO);
	glBindVertexArray(objVAO);
	glBindBuffer(GL_ARRAY_BUFFER, objVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * objData.vertexes.size() , &objData.vertexes[0], GL_STATIC_DRAW);	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, objData.indices.size() * sizeof(unsigned int),
		     &objData.indices[0], GL_STATIC_DRAW);
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glBindVertexArray(0);

	return objVAO;
}


inline void update_scene_objects() {
	auto entity = active_scene->entities.begin();
	auto end = active_scene->entities.end();
	for (entity; entity < end; entity++) {
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
	window = glfwCreateWindow(viewportWidth, viewportHeight, "Ravenous", NULL, NULL);
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
	glViewport(0, 0, viewportWidth, viewportHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, onMouseMove);
	glfwSetScrollCallback(window, onMouseScroll);
	glfwSetMouseButtonCallback(window, onMouseBtn);

	if (debug) {
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	}

}

void render_scene_lights() {
	auto it = active_scene->pointLights.begin();
	auto end = active_scene->pointLights.end();
	for (it; it != end; it++) {
		quad_shader.use();
		quad_shader.setMatrix4("view", active_camera.View4x4);
		quad_shader.setMatrix4("projection", active_camera.Projection4x4);

		vec3 light_norm = active_camera.Position - it->position;
		float angle_pitch = atan(light_norm.x / light_norm.z);

		glm::mat4 model_m = glm::translate(mat4identity, it->position);
		model_m = rotate(model_m, angle_pitch, active_camera.Up);
		model_m = glm::scale(model_m, vec3(light_icons_scaling, light_icons_scaling * 1.5f, light_icons_scaling));
		quad_shader.setMatrix4("model", model_m);

		glBindVertexArray(quad_vao);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, quad_lightbulb_texture);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
}


void render_scene() {
	auto entity_ptr = active_scene->entities.begin();
	for (entity_ptr; entity_ptr != active_scene->entities.end(); entity_ptr++) {
		entity_ptr->shader->use();
		auto point_light_ptr = active_scene->pointLights.begin();
		int point_light_count = 0;
		for (point_light_ptr; point_light_ptr != active_scene->pointLights.end(); point_light_ptr++) {
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
			resetMouseCoords = true;
		}
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
			global_shininess -= 10 * deltaTime;
			if (global_shininess < 1)
				global_shininess = 1;
		}
		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
			global_shininess += 10 * deltaTime;



		// Toggle GUI (substitute all checks for just one varible holding last key pressed (or keys)
		// then when released, set it to null or zero or something.
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !GUI_btn_down) {
			GUI_btn_down = true;
			if (show_GUI)
				show_GUI = false;
			else show_GUI = true;
		}
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE && GUI_btn_down)
			GUI_btn_down = false;

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			active_camera = cameraList[0];
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			active_camera = cameraList[1];
		}

	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !keyComboPressed) {
		if (moveMode) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			moveMode = false;
			keyComboPressed = true;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			moveMode = true;
			resetMouseCoords = true;
			keyComboPressed = true;
		}
	}
	// This solution will only work while i have only one key combo implemented (i guess)
	if (keyComboPressed) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
			keyComboPressed = false;
	}

}

void onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
	// if (editor_mode) {
	// 	editor_process_input_mouse_move(xpos, ypos);
	// }

   if (moveMode || (editor_controls.is_mouse_drag && !entity_controls.is_dragging_entity)) {
      // 'teleports' stored coordinates to current mouse coordinates
      if (resetMouseCoords) {
         lastXMouseCoord = xpos;
         lastYMouseCoord = ypos;
         resetMouseCoords = false;
      }

      // calculates offsets
      float xoffset = xpos - lastXMouseCoord;
      float yoffset = lastYMouseCoord - ypos;
      lastXMouseCoord = xpos;
      lastYMouseCoord = ypos;

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

   currentMouseX = xpos;
	currentMouseY = ypos;

	// mouse dragging controls
	if (editor_controls.is_mouse_left_btn_press
		&& (abs(editor_controls.mouse_btn_down_x - currentMouseX) > 2
			|| abs(editor_controls.mouse_btn_down_y - currentMouseY) > 2)) {
		editor_controls.is_mouse_drag = true;
	}
}

void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
	// if (editor_mode) {
	// 	if (!ImGui::GetIO().WantCaptureMouse) {
	// 		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
	// 			if (active_camera.FOVy <= 45.0f && active_camera.FOVy >= 1.0f)
	// 				active_camera.FOVy -= yoffset * 3;
	// 			if (active_camera.FOVy < 1.0f)
	// 				active_camera.FOVy = 1.0f;
	// 			if (active_camera.FOVy > 45.0f)
	// 				active_camera.FOVy = 45.0f;
	// 		}
	// 		else {
				active_camera.Position += (float)(3 * yoffset) * active_camera.Front;
	// 		}
	// 	}
	// }
}

void onMouseBtn(GLFWwindow* window, int button, int action, int mods) {
	// if (editor_mode) {
	// 	editor_process_input_mouse_btn(button, action);
	// }

   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      editor_controls.is_mouse_left_btn_press = true;
      resetMouseCoords = true;
   }
   else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      cout << "left_btn_release" << endl;
      editor_controls.is_mouse_left_btn_press = false;
      cout << editor_controls.is_mouse_drag << endl;
      if (!editor_controls.is_mouse_drag && !moveMode) {


      }
      editor_controls.press_release_toggle = GLFW_RELEASE;
      editor_controls.is_mouse_drag = false;
      entity_controls.is_dragging_entity = false;
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
