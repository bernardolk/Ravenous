#pragma once

#include "engine/core/core.h"

#define glCheckError() glCheckError_(__FILE__, __LINE__)


void setup_GLFW(bool debug);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLenum glCheckError_(const char* file, int line);
void SetupGL();
