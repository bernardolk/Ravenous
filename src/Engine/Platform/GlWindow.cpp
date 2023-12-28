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
	auto* DisplayState = GlobalDisplayState::Get();
	auto* NewWindow = glfwCreateWindow(GlobalDisplayState::ViewportWidth, GlobalDisplayState::ViewportHeight, "Ravenous", nullptr, nullptr);
	if (NewWindow == nullptr)
	{
		Log("Failed to create GLFW window");
		glfwTerminate();
	}

	auto* Window = DisplayState->Initialize(NewWindow);
	glfwMakeContextCurrent(Window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		Log("Failed to initialize GLAD");
	}

	// Setups openGL viewport
	glViewport(0, 0, GlobalDisplayState::ViewportWidth, GlobalDisplayState::ViewportHeight);
	glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);
	glfwSetCursorPosCallback(Window, OnMouseMove);
	glfwSetScrollCallback(Window, OnMouseScroll);
	glfwSetMouseButtonCallback(Window, OnMouseBtn);

#ifdef DEBUG_BUILD
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
}

void FramebufferSizeCallback(GLFWwindow* Window, int Width, int Height)
{
	glViewport(0, 0, Width, Height);
}

GLenum GLCheckError(const char* File, int Line)
{
	GLenum ErrorCode;
	while ((ErrorCode = glGetError()) != GL_NO_ERROR)
	{
		string ErrorString;
		switch (ErrorCode)
		{
			case GL_INVALID_ENUM:
				ErrorString = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				ErrorString = "INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				ErrorString = "INVALID_OPERATION";
				break;
			//case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
			//case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:
				ErrorString = "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				ErrorString = "INVALID_FRAMEBUFFER_OPERATION";
				break;
			default:
				break;
		}
		printf(ErrorString.c_str());
		Log(" at file '%s' line: %i", File, Line);
	}
	return ErrorCode;
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
