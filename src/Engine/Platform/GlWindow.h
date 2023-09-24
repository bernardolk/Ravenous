#pragma once

#include "engine/core/core.h"
#ifndef GLAD_INCL
#define GLAD_INCL
#include <glad/glad.h>
#endif
#include <glfw3.h>

#define GL_CHECK_ERROR() glCheckError_(__FILE__, __LINE__)

void SetupGLFW();
void SetupGL();

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
GLenum GLCheckError(const char* file, int line);
