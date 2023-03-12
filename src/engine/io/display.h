#pragma once

#include "engine/core/core.h"

struct GlobalDisplayConfig
{
	GLFWwindow* window;
	constexpr inline static float viewport_width = 1980;
	constexpr inline static float viewport_height = 1080;

	static GlobalDisplayConfig* Get()
	{
		static GlobalDisplayConfig instance;
		return &instance;
	}
	static GLFWwindow* GetWindow()
	{
		auto* GDC = Get();
		return GDC->window;
	}
};
