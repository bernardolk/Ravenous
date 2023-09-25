#pragma once

#include "engine/core/core.h"

struct RInputFlags
{
	uint64 KeyPress = 0;
	uint64 KeyRelease = 0;
	uint8 MousePress = 0;
	uint8 MouseRelease = 0;
};

void OnMouseBtn(GLFWwindow* Window, int Button, int Action, int Mods);
void OnMouseMove(GLFWwindow* Window, double Xpos, double Ypos);
uint64 ProcessKeyboardInputKeyPress(GLFWwindow* Window);
uint64 ProcessKeyboardInputKeyRelease(GLFWwindow* Window);
void OnMouseScroll(GLFWwindow* Window, double Xoffset, double Yoffset);
struct RInputFlags StartInputPhase();
bool Pressed(RInputFlags Flags, uint64 Key);
bool PressedOnce(RInputFlags Flags, uint64 Key);
bool PressedOnly(RInputFlags Flags, uint64 Key);
void CheckMouseClickHold();
void ResetInputFlags(RInputFlags Flags);

inline constexpr uint64 KeyQ = 1LL << 0;
inline constexpr uint64 KeyW = 1LL << 1;
inline constexpr uint64 KeyE = 1LL << 2;
inline constexpr uint64 KeyR = 1LL << 3;
inline constexpr uint64 KeyT = 1LL << 4;
inline constexpr uint64 KeyY = 1LL << 5;
inline constexpr uint64 KeyU = 1LL << 6;
inline constexpr uint64 KeyI = 1LL << 7;
inline constexpr uint64 KeyO = 1LL << 8;
inline constexpr uint64 KeyP = 1LL << 9;
inline constexpr uint64 KeyA = 1LL << 10;
inline constexpr uint64 KeyS = 1LL << 11;
inline constexpr uint64 KeyD = 1LL << 12;
inline constexpr uint64 KeyF = 1LL << 13;
inline constexpr uint64 KeyG = 1LL << 14;
inline constexpr uint64 KeyH = 1LL << 15;
inline constexpr uint64 KeyJ = 1LL << 16;
inline constexpr uint64 KeyK = 1LL << 17;
inline constexpr uint64 KeyL = 1LL << 18;
inline constexpr uint64 KeyZ = 1LL << 19;
inline constexpr uint64 KeyX = 1LL << 20;
inline constexpr uint64 KeyC = 1LL << 21;
inline constexpr uint64 KeyV = 1LL << 22;
inline constexpr uint64 KeyB = 1LL << 23;
inline constexpr uint64 KeyN = 1LL << 24;
inline constexpr uint64 KeyM = 1LL << 25;
inline constexpr uint64 Key0 = 1LL << 26;
inline constexpr uint64 Key1 = 1LL << 27;
inline constexpr uint64 Key2 = 1LL << 28;
inline constexpr uint64 Key3 = 1LL << 29;
inline constexpr uint64 Key4 = 1LL << 30;
inline constexpr uint64 Key5 = 1LL << 31;
inline constexpr uint64 Key6 = 1LL << 32;
inline constexpr uint64 Key7 = 1LL << 33;
inline constexpr uint64 Key8 = 1LL << 34;
inline constexpr uint64 Key9 = 1LL << 35;
inline constexpr uint64 KeyLeft = 1LL << 36;
inline constexpr uint64 KeyRight = 1LL << 37;
inline constexpr uint64 KeyUp = 1LL << 38;
inline constexpr uint64 KeyDown = 1LL << 39;
inline constexpr uint64 KeySpace = 1LL << 40;
inline constexpr uint64 KeyEsc = 1LL << 41;
inline constexpr uint64 KeyLeftShift = 1LL << 42;
inline constexpr uint64 KeyLeftCtrl = 1LL << 43;
inline constexpr uint64 KeyGraveTick = 1LL << 44;
inline constexpr uint64 KeyEnter = 1LL << 45;
inline constexpr uint64 KeyBackspace = 1LL << 46;
inline constexpr uint64 KeyComma = 1LL << 47;
inline constexpr uint64 KeyPeriod = 1LL << 48;
inline constexpr uint64 KeyDelete = 1LL << 49;
inline constexpr uint64 KeyNone = 1LL << 50;

inline constexpr uint16 MouseLbClick = 1 << 0;
inline constexpr uint16 MouseRbClick = 1 << 1;
inline constexpr uint16 MouseRbDragging = 1 << 2;
inline constexpr uint16 MouseLbHold = 1 << 3;
inline constexpr uint16 MouseRbHold = 1 << 4;
inline constexpr uint16 MouseLbDragging = 1 << 5;
