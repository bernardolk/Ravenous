#pragma once

#include "engine/core/core.h"

void OnMouseBtn(GLFWwindow* window, int button, int action, int mods);
void OnMouseMove(GLFWwindow* window, double xpos, double ypos);
u64 ProcessKeyboardInputKeyPress(GLFWwindow* window);
u64 ProcessKeyboardInputKeyRelease(GLFWwindow* window);
void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);
struct InputFlags InputPhase();
bool Pressed(InputFlags flags, u64 key);
bool PressedOnce(InputFlags flags, u64 key);
bool PressedOnly(InputFlags flags, u64 key);
void CheckMouseClickHold();
void ResetInputFlags(InputFlags flags);

struct InputFlags
{
	u64 key_press = 0;
	u64 key_release = 0;
	u8 mouse_press = 0;
	u8 mouse_release = 0;
};

inline constexpr u64 KEY_Q = 1LL << 0;
inline constexpr u64 KEY_W = 1LL << 1;
inline constexpr u64 KEY_E = 1LL << 2;
inline constexpr u64 KEY_R = 1LL << 3;
inline constexpr u64 KEY_T = 1LL << 4;
inline constexpr u64 KEY_Y = 1LL << 5;
inline constexpr u64 KEY_U = 1LL << 6;
inline constexpr u64 KEY_I = 1LL << 7;
inline constexpr u64 KEY_O = 1LL << 8;
inline constexpr u64 KEY_P = 1LL << 9;
inline constexpr u64 KEY_A = 1LL << 10;
inline constexpr u64 KEY_S = 1LL << 11;
inline constexpr u64 KEY_D = 1LL << 12;
inline constexpr u64 KEY_F = 1LL << 13;
inline constexpr u64 KEY_G = 1LL << 14;
inline constexpr u64 KEY_H = 1LL << 15;
inline constexpr u64 KEY_J = 1LL << 16;
inline constexpr u64 KEY_K = 1LL << 17;
inline constexpr u64 KEY_L = 1LL << 18;
inline constexpr u64 KEY_Z = 1LL << 19;
inline constexpr u64 KEY_X = 1LL << 20;
inline constexpr u64 KEY_C = 1LL << 21;
inline constexpr u64 KEY_V = 1LL << 22;
inline constexpr u64 KEY_B = 1LL << 23;
inline constexpr u64 KEY_N = 1LL << 24;
inline constexpr u64 KEY_M = 1LL << 25;
inline constexpr u64 KEY_0 = 1LL << 26;
inline constexpr u64 KEY_1 = 1LL << 27;
inline constexpr u64 KEY_2 = 1LL << 28;
inline constexpr u64 KEY_3 = 1LL << 29;
inline constexpr u64 KEY_4 = 1LL << 30;
inline constexpr u64 KEY_5 = 1LL << 31;
inline constexpr u64 KEY_6 = 1LL << 32;
inline constexpr u64 KEY_7 = 1LL << 33;
inline constexpr u64 KEY_8 = 1LL << 34;
inline constexpr u64 KEY_9 = 1LL << 35;
inline constexpr u64 KEY_LEFT = 1LL << 36;
inline constexpr u64 KEY_RIGHT = 1LL << 37;
inline constexpr u64 KEY_UP = 1LL << 38;
inline constexpr u64 KEY_DOWN = 1LL << 39;
inline constexpr u64 KEY_SPACE = 1LL << 40;
inline constexpr u64 KEY_ESC = 1LL << 41;
inline constexpr u64 KEY_LEFT_SHIFT = 1LL << 42;
inline constexpr u64 KEY_LEFT_CTRL = 1LL << 43;
inline constexpr u64 KEY_GRAVE_TICK = 1LL << 44;
inline constexpr u64 KEY_ENTER = 1LL << 45;
inline constexpr u64 KEY_BACKSPACE = 1LL << 46;
inline constexpr u64 KEY_COMMA = 1LL << 47;
inline constexpr u64 KEY_PERIOD = 1LL << 48;
inline constexpr u64 KEY_DELETE = 1LL << 49;

inline constexpr u16 MOUSE_LB_CLICK = 1 << 0;
inline constexpr u16 MOUSE_RB_CLICK = 1 << 1;
inline constexpr u16 MOUSE_RB_DRAGGING = 1 << 2;
inline constexpr u16 MOUSE_LB_HOLD = 1 << 3;
inline constexpr u16 MOUSE_RB_HOLD = 1 << 4;
inline constexpr u16 MOUSE_LB_DRAGGING = 1 << 5;