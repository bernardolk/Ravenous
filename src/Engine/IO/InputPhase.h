#pragma once

#include "Input.h"
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
bool Pressed(RInputFlags Flags, NKeyInput Key);
bool PressedOnce(RInputFlags Flags, NKeyInput Key);
bool PressedOnly(RInputFlags Flags, NKeyInput Key);
void CheckMouseClickHold();
void ResetInputFlags(RInputFlags Flags);
