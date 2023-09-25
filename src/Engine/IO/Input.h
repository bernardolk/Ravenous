#pragma once

#include "engine/core/core.h"

struct MouseCoordinates
{
	double LastX = 0;
	double LastY = 0;
	double ClickX;
	double ClickY;
	double X;
	double Y;
};

struct GlobalInputInfo
{
	DeclSingleton(GlobalInputInfo)
	bool ForgetLastMouseCoords = true;
	MouseCoordinates MouseCoords;
	uint64 KeyState = 0;
	uint8 MouseState = 0;
	bool BlockMouseMove = false;
};
