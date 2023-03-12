#pragma once

#include "engine/core/core.h"

#define GL_CHECK_ERROR() glCheckError_(__FILE__, __LINE__)


void SetupGLFW(bool debug);
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
GLenum GLCheckError(const char* file, int line);
void SetupGL();
