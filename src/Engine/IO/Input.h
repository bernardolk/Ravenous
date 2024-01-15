#pragma once

#include "engine/core/core.h"

// @TODO: Encapsulate value access for enum class key flag values, as casting to uint64 / uint16 is error prone, redundant,
// and maintanance headache in case we need to change the length of the flags container

/* ----------------------
 * ~	Input flags
 * ---------------------- */
enum class NKeyInput : uint64
{
	// 1st byte
	KeyNone = 1ULL << 0,
	KeyQ = 1ULL << 1,
	KeyW = 1ULL << 2,
	KeyE = 1ULL << 3,
	KeyR = 1ULL << 4,
	KeyT = 1ULL << 5,
	KeyY = 1ULL << 6,
	KeyU = 1ULL << 7,

	// 2nd byte
	KeyI = 1ULL << 8,
	KeyO = 1ULL << 9,
	KeyP = 1ULL << 10,
	KeyA = 1ULL << 11,
	KeyS = 1ULL << 12,
	KeyD = 1ULL << 13,
	KeyF = 1ULL << 14,
	KeyG = 1ULL << 15,

	// 3rd byte
	KeyH = 1ULL << 16,
	KeyJ = 1ULL << 17,
	KeyK = 1ULL << 18,
	KeyL = 1ULL << 19,
	KeyZ = 1ULL << 20,
	KeyX = 1ULL << 21,
	KeyC = 1ULL << 22,
	KeyV = 1ULL << 23,

	// 4th byte
	KeyB = 1ULL << 24,
	KeyN = 1ULL << 25,
	KeyM = 1ULL << 26,
	Key0 = 1ULL << 27,
	Key1 = 1ULL << 28,
	Key2 = 1ULL << 29,
	Key3 = 1ULL << 30,
	Key4 = 1ULL << 31,

	// 5th byte
	Key5 = 1ULL << 32,
	Key6 = 1ULL << 33,
	Key7 = 1ULL << 34,
	Key8 = 1ULL << 35,
	Key9 = 1ULL << 36,
	KeyLeft = 1ULL << 37,
	KeyRight = 1ULL << 38,
	KeyUp = 1ULL << 39,

	// 6th byte
	KeyDown = 1ULL << 40,
	KeySpace = 1ULL << 41,
	KeyEsc = 1ULL << 42,
	KeyLeftShift = 1ULL << 43,
	KeyLeftCtrl = 1ULL << 44,
	KeyGraveTick = 1ULL << 45,
	KeyEnter = 1ULL << 46,
	KeyBackspace = 1ULL << 47,

	// 7th byte
	KeyComma = 1ULL << 48,
	KeyPeriod = 1ULL << 49,
	KeyDelete = 1ULL << 50,
	KeyPause = 1ULL << 51,
	KeyEnd = 1ULL << 52,
};

enum class NMouseInput : uint16
{
	LeftButtonClick = 1 << 0,
	RightButtonClick = 1 << 1,
	RightButtonDragging = 1 << 2,
	LeftButtonHold = 1 << 3,
	RightButtonHold = 1 << 4,
	LeftButtonDragging = 1 << 5,
};

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
	static GlobalInputInfo* Get() 
	{ 
		static GlobalInputInfo Instance{};
		return &Instance;
	}
	
	bool ForgetLastMouseCoords = true;
	MouseCoordinates MouseCoords;
	uint64 KeyState = 0;
	uint8 MouseState = 0;
	bool BlockMouseMove = false;
};
