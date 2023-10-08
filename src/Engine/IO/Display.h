#pragma once

#include "Engine/Core/Core.h"

// @TODO: This code leaks platform-dependent types to the rest of the codebase, fixit. 
struct GlobalDisplayState
{
	static GlobalDisplayState* Get()
	{
		static GlobalDisplayState Instance{};
		return &Instance;
	}
	
	constexpr inline static float ViewportWidth = 1980;
	constexpr inline static float ViewportHeight = 1080;

	GLFWwindow* GetWindow() const
	{
		assert(Window);
		return Window;
	}
	GLFWwindow* Initialize(GLFWwindow* NewWindow)
	{
		Window = NewWindow;
		return GetWindow();
	}

private:
	GLFWwindow* Window = nullptr;
};
