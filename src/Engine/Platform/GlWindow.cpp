#include "GlWindow.h"
#include "Engine/IO/InputPhase.h"
#include "Engine/IO/Display.h"

void SetupGLFW()
{
	// Setup the window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Creates the window
	auto* display_state = GlobalDisplayState::Get();
	auto* new_window = glfwCreateWindow(GlobalDisplayState::viewport_width, GlobalDisplayState::viewport_height, "Ravenous", nullptr, nullptr);
	if (new_window == nullptr)
	{
		print("Failed to create GLFW window");
		glfwTerminate();
	}
	
	auto* window = display_state->Initialize(new_window);
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		print("Failed to initialize GLAD");
	}

	// Setups openGL viewport
	glViewport(0, 0, GlobalDisplayState::viewport_width, GlobalDisplayState::viewport_height);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetCursorPosCallback(window, OnMouseMove);
	glfwSetScrollCallback(window, OnMouseScroll);
	glfwSetMouseButtonCallback(window, OnMouseBtn);

#ifdef DEBUG_BUILD
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLenum GLCheckError(const char* file, int line)
{
	GLenum error_code;
	while ((error_code = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (error_code)
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
			default:
				break;
		}
		printf(error.c_str());
		print(" at file '%s' line: %i", file, line);
	}
	return error_code;
}

void SetupGL()
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
