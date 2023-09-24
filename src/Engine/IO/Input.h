#pragma once

#include "engine/core/core.h"

struct MouseCoordinates
{
	double last_x = 0;
	double last_y = 0;
	double click_x;
	double click_y;
	double x;
	double y;
};

struct GlobalInputInfo
{
	DeclSingleton(GlobalInputInfo)
	
	bool forget_last_mouse_coords = true;
	MouseCoordinates mouse_coords;
	uint64 key_state = 0;
	uint8 mouse_state = 0;
	bool block_mouse_move = false;
};
