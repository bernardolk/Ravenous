#include "gl.h"

#include <glad.h>
#include <glfw3.h>
#include "in_phase.h"
#include "engine/io/display.h"


void setup_GLFW(bool debug)
{
	// Setup the window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Creates the window
	auto* GDC = GlobalDisplayConfig::Get();
	GDC->window = glfwCreateWindow(GlobalDisplayConfig::viewport_width, GlobalDisplayConfig::viewport_height, "Ravenous", nullptr, nullptr);
	if(GDC->window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(GDC->window);

	if(!gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Setups openGL viewport
	glViewport(0, 0, GlobalDisplayConfig::viewport_width, GlobalDisplayConfig::viewport_height);
	glfwSetFramebufferSizeCallback(GDC->window, framebuffer_size_callback);
	glfwSetCursorPosCallback(GDC->window, on_mouse_move);
	glfwSetScrollCallback(GDC->window, on_mouse_scroll);
	glfwSetMouseButtonCallback(GDC->window, on_mouse_btn);

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