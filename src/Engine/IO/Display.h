#pragma once

#include "Engine/Core/Core.h"

// @TODO: This code leaks platform-dependent types to the rest of the codebase, fixit. 
struct GlobalDisplayState
{
	DeclSingleton(GlobalDisplayState)
	
	constexpr inline static float viewport_width = 1980;
	constexpr inline static float viewport_height = 1080;

	GLFWwindow* GetWindow() const { assert(window); return window; }
	GLFWwindow* Initialize(GLFWwindow* new_window) { window = new_window; return GetWindow(); }

private:
	GLFWwindow* window = nullptr;
};
