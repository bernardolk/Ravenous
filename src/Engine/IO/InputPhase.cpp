#include "InputPhase.h"
#include "engine/io/display.h"
#include "engine/io/input.h"
#include "editor/EditorState.h"
#include <glfw3.h>
#include <imgui.h>
#include "engine/camera/camera.h"


RInputFlags StartInputPhase()
{
	// first, check if last frame we had a click, if so, 
	// se it as btn hold (so we dont register clicks more than one time)
	// @TODO: maybe the best approach here is to pool it like we do with the keys
	// instead of using a callback. If so, need to check whether we would need
	// sticky mouse click input config set to true or not
	CheckMouseClickHold();
	// then respond to all glfw callbacks
	glfwPollEvents();
	// set the flags and return
	auto* GDC = GlobalDisplayState::Get();
	auto KeyPressFlags = ProcessKeyboardInputKeyPress(GDC->GetWindow());
	auto KeyReleaseFlags = ProcessKeyboardInputKeyRelease(GDC->GetWindow());
	return RInputFlags{KeyPressFlags, KeyReleaseFlags};
}


uint64 ProcessKeyboardInputKeyPress(GLFWwindow* Window)
{
	uint64 Flags = 0;

	if (glfwGetKey(Window, GLFW_KEY_Q) == GLFW_PRESS)
		Flags = Flags | KeyQ;

	if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
		Flags = Flags | KeyW;

	if (glfwGetKey(Window, GLFW_KEY_E) == GLFW_PRESS)
		Flags = Flags | KeyE;

	if (glfwGetKey(Window, GLFW_KEY_R) == GLFW_PRESS)
		Flags = Flags | KeyR;

	if (glfwGetKey(Window, GLFW_KEY_T) == GLFW_PRESS)
		Flags = Flags | KeyT;

	if (glfwGetKey(Window, GLFW_KEY_Y) == GLFW_PRESS)
		Flags = Flags | KeyY;

	if (glfwGetKey(Window, GLFW_KEY_U) == GLFW_PRESS)
		Flags = Flags | KeyU;

	if (glfwGetKey(Window, GLFW_KEY_I) == GLFW_PRESS)
		Flags = Flags | KeyI;

	if (glfwGetKey(Window, GLFW_KEY_O) == GLFW_PRESS)
		Flags = Flags | KeyO;

	if (glfwGetKey(Window, GLFW_KEY_P) == GLFW_PRESS)
		Flags = Flags | KeyP;

	if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
		Flags = Flags | KeyA;

	if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
		Flags = Flags | KeyS;

	if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
		Flags = Flags | KeyD;

	if (glfwGetKey(Window, GLFW_KEY_F) == GLFW_PRESS)
		Flags = Flags | KeyF;

	if (glfwGetKey(Window, GLFW_KEY_G) == GLFW_PRESS)
		Flags = Flags | KeyG;

	if (glfwGetKey(Window, GLFW_KEY_H) == GLFW_PRESS)
		Flags = Flags | KeyH;

	if (glfwGetKey(Window, GLFW_KEY_J) == GLFW_PRESS)
		Flags = Flags | KeyJ;

	if (glfwGetKey(Window, GLFW_KEY_K) == GLFW_PRESS)
		Flags = Flags | KeyK;

	if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_PRESS)
		Flags = Flags | KeyL;

	if (glfwGetKey(Window, GLFW_KEY_Z) == GLFW_PRESS)
		Flags = Flags | KeyZ;

	if (glfwGetKey(Window, GLFW_KEY_X) == GLFW_PRESS)
		Flags = Flags | KeyX;

	if (glfwGetKey(Window, GLFW_KEY_C) == GLFW_PRESS)
		Flags = Flags | KeyC;

	if (glfwGetKey(Window, GLFW_KEY_V) == GLFW_PRESS)
		Flags = Flags | KeyV;

	if (glfwGetKey(Window, GLFW_KEY_B) == GLFW_PRESS)
		Flags = Flags | KeyB;

	if (glfwGetKey(Window, GLFW_KEY_N) == GLFW_PRESS)
		Flags = Flags | KeyN;

	if (glfwGetKey(Window, GLFW_KEY_M) == GLFW_PRESS)
		Flags = Flags | KeyM;

	if (glfwGetKey(Window, GLFW_KEY_0) == GLFW_PRESS)
		Flags = Flags | Key0;

	if (glfwGetKey(Window, GLFW_KEY_1) == GLFW_PRESS)
		Flags = Flags | Key1;

	if (glfwGetKey(Window, GLFW_KEY_2) == GLFW_PRESS)
		Flags = Flags | Key2;

	if (glfwGetKey(Window, GLFW_KEY_3) == GLFW_PRESS)
		Flags = Flags | Key3;

	if (glfwGetKey(Window, GLFW_KEY_4) == GLFW_PRESS)
		Flags = Flags | Key4;

	if (glfwGetKey(Window, GLFW_KEY_5) == GLFW_PRESS)
		Flags = Flags | Key5;

	if (glfwGetKey(Window, GLFW_KEY_6) == GLFW_PRESS)
		Flags = Flags | Key6;

	if (glfwGetKey(Window, GLFW_KEY_7) == GLFW_PRESS)
		Flags = Flags | Key7;

	if (glfwGetKey(Window, GLFW_KEY_8) == GLFW_PRESS)
		Flags = Flags | Key8;

	if (glfwGetKey(Window, GLFW_KEY_9) == GLFW_PRESS)
		Flags = Flags | Key9;

	if (glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		Flags = Flags | KeyLeftShift;

	if (glfwGetKey(Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		Flags = Flags | KeyLeftCtrl;

	if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		Flags = Flags | KeyEsc;

	if (glfwGetKey(Window, GLFW_KEY_UP) == GLFW_PRESS)
		Flags = Flags | KeyUp;

	if (glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_PRESS)
		Flags = Flags | KeyDown;

	if (glfwGetKey(Window, GLFW_KEY_LEFT) == GLFW_PRESS)
		Flags = Flags | KeyLeft;

	if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		Flags = Flags | KeyRight;

	if (glfwGetKey(Window, GLFW_KEY_SPACE) == GLFW_PRESS)
		Flags = Flags | KeySpace;

	if (glfwGetKey(Window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
		Flags = Flags | KeyGraveTick;

	if (glfwGetKey(Window, GLFW_KEY_ENTER) == GLFW_PRESS)
		Flags = Flags | KeyEnter;

	if (glfwGetKey(Window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
		Flags = Flags | KeyBackspace;

	if (glfwGetKey(Window, GLFW_KEY_COMMA) == GLFW_PRESS)
		Flags = Flags | KeyComma;

	if (glfwGetKey(Window, GLFW_KEY_PERIOD) == GLFW_PRESS)
		Flags = Flags | KeyPeriod;

	if (glfwGetKey(Window, GLFW_KEY_DELETE) == GLFW_PRESS)
		Flags = Flags | KeyDelete;

	return Flags;
}


uint64 ProcessKeyboardInputKeyRelease(GLFWwindow* Window)
{
	uint64 Flags = 0;

	if (glfwGetKey(Window, GLFW_KEY_Q) == GLFW_RELEASE)
		Flags = Flags | KeyQ;

	if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_RELEASE)
		Flags = Flags | KeyW;

	if (glfwGetKey(Window, GLFW_KEY_E) == GLFW_RELEASE)
		Flags = Flags | KeyE;

	if (glfwGetKey(Window, GLFW_KEY_R) == GLFW_RELEASE)
		Flags = Flags | KeyR;

	if (glfwGetKey(Window, GLFW_KEY_T) == GLFW_RELEASE)
		Flags = Flags | KeyT;

	if (glfwGetKey(Window, GLFW_KEY_Y) == GLFW_RELEASE)
		Flags = Flags | KeyY;

	if (glfwGetKey(Window, GLFW_KEY_U) == GLFW_RELEASE)
		Flags = Flags | KeyU;

	if (glfwGetKey(Window, GLFW_KEY_I) == GLFW_RELEASE)
		Flags = Flags | KeyI;

	if (glfwGetKey(Window, GLFW_KEY_O) == GLFW_RELEASE)
		Flags = Flags | KeyO;

	if (glfwGetKey(Window, GLFW_KEY_P) == GLFW_RELEASE)
		Flags = Flags | KeyP;

	if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_RELEASE)
		Flags = Flags | KeyA;

	if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_RELEASE)
		Flags = Flags | KeyS;

	if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_RELEASE)
		Flags = Flags | KeyD;

	if (glfwGetKey(Window, GLFW_KEY_F) == GLFW_RELEASE)
		Flags = Flags | KeyF;

	if (glfwGetKey(Window, GLFW_KEY_G) == GLFW_RELEASE)
		Flags = Flags | KeyG;

	if (glfwGetKey(Window, GLFW_KEY_H) == GLFW_RELEASE)
		Flags = Flags | KeyH;

	if (glfwGetKey(Window, GLFW_KEY_J) == GLFW_RELEASE)
		Flags = Flags | KeyJ;

	if (glfwGetKey(Window, GLFW_KEY_K) == GLFW_RELEASE)
		Flags = Flags | KeyK;

	if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_RELEASE)
		Flags = Flags | KeyL;

	if (glfwGetKey(Window, GLFW_KEY_Z) == GLFW_RELEASE)
		Flags = Flags | KeyZ;

	if (glfwGetKey(Window, GLFW_KEY_X) == GLFW_RELEASE)
		Flags = Flags | KeyX;

	if (glfwGetKey(Window, GLFW_KEY_C) == GLFW_RELEASE)
		Flags = Flags | KeyC;

	if (glfwGetKey(Window, GLFW_KEY_V) == GLFW_RELEASE)
		Flags = Flags | KeyV;

	if (glfwGetKey(Window, GLFW_KEY_B) == GLFW_RELEASE)
		Flags = Flags | KeyB;

	if (glfwGetKey(Window, GLFW_KEY_N) == GLFW_RELEASE)
		Flags = Flags | KeyN;

	if (glfwGetKey(Window, GLFW_KEY_M) == GLFW_RELEASE)
		Flags = Flags | KeyM;

	if (glfwGetKey(Window, GLFW_KEY_0) == GLFW_RELEASE)
		Flags = Flags | Key0;

	if (glfwGetKey(Window, GLFW_KEY_1) == GLFW_RELEASE)
		Flags = Flags | Key1;

	if (glfwGetKey(Window, GLFW_KEY_2) == GLFW_RELEASE)
		Flags = Flags | Key2;

	if (glfwGetKey(Window, GLFW_KEY_3) == GLFW_RELEASE)
		Flags = Flags | Key3;

	if (glfwGetKey(Window, GLFW_KEY_4) == GLFW_RELEASE)
		Flags = Flags | Key4;

	if (glfwGetKey(Window, GLFW_KEY_5) == GLFW_RELEASE)
		Flags = Flags | Key5;

	if (glfwGetKey(Window, GLFW_KEY_6) == GLFW_RELEASE)
		Flags = Flags | Key6;

	if (glfwGetKey(Window, GLFW_KEY_7) == GLFW_RELEASE)
		Flags = Flags | Key7;

	if (glfwGetKey(Window, GLFW_KEY_8) == GLFW_RELEASE)
		Flags = Flags | Key8;

	if (glfwGetKey(Window, GLFW_KEY_9) == GLFW_RELEASE)
		Flags = Flags | Key9;

	if (glfwGetKey(Window, GLFW_KEY_UP) == GLFW_RELEASE)
		Flags = Flags | KeyUp;

	if (glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_RELEASE)
		Flags = Flags | KeyDown;

	if (glfwGetKey(Window, GLFW_KEY_LEFT) == GLFW_RELEASE)
		Flags = Flags | KeyLeft;

	if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
		Flags = Flags | KeyRight;

	if (glfwGetKey(Window, GLFW_KEY_SPACE) == GLFW_RELEASE)
		Flags = Flags | KeySpace;

	if (glfwGetKey(Window, GLFW_KEY_GRAVE_ACCENT) == GLFW_RELEASE)
		Flags = Flags | KeyGraveTick;

	if (glfwGetKey(Window, GLFW_KEY_ENTER) == GLFW_RELEASE)
		Flags = Flags | KeyEnter;

	if (glfwGetKey(Window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE)
		Flags = Flags | KeyBackspace;

	if (glfwGetKey(Window, GLFW_KEY_COMMA) == GLFW_RELEASE)
		Flags = Flags | KeyComma;

	if (glfwGetKey(Window, GLFW_KEY_PERIOD) == GLFW_RELEASE)
		Flags = Flags | KeyPeriod;

	if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
		Flags = Flags | KeyEsc;

	if (glfwGetKey(Window, GLFW_KEY_DELETE) == GLFW_RELEASE)
		Flags = Flags | KeyDelete;

	return Flags;
}


void OnMouseMove(GLFWwindow* Window, double Xpos, double Ypos)
{
	auto* GII = GlobalInputInfo::Get();
	auto* ES = REditorState::Get();

	if (REditorState::IsInEditorMode() && ImGui::GetIO().WantCaptureMouse)
		return;

	// activates mouse dragging if clicking and current mouse position has changed a certain ammount
	if (!(GII->MouseState & MouseRbDragging) && GII->MouseState & MouseRbHold)
	{
		auto OffsetFromClickX = abs(GII->MouseCoords.ClickX - GII->MouseCoords.X);
		auto OffsetFromClickY = abs(GII->MouseCoords.ClickY - GII->MouseCoords.Y);
		if (OffsetFromClickX > 2 || OffsetFromClickY > 2)
		{
			GII->MouseState |= MouseRbDragging;
		}
	}

	// do the same for LB
	if (!(GII->MouseState & MouseLbDragging) && GII->MouseState & MouseLbHold)
	{
		auto OffsetFromClickX = abs(GII->MouseCoords.ClickX - GII->MouseCoords.X);
		auto OffsetFromClickY = abs(GII->MouseCoords.ClickY - GII->MouseCoords.Y);
		if (OffsetFromClickX > 2 || OffsetFromClickY > 2)
		{
			GII->MouseState |= MouseLbDragging;
		}
	}

	// @todo: should refactor this out of here
	// MOVE CAMERA WITH MOUSE IF APPROPRIATE
	if (ES->CurrentMode == REditorState::ProgramMode::Game || (GII->MouseState & MouseRbDragging))
	{
		if (GII->BlockMouseMove)
			return;

		// 'teleports' stored coordinates to current mouse coordinates
		if (GII->ForgetLastMouseCoords)
		{
			GII->MouseCoords.LastX = Xpos;
			GII->MouseCoords.LastY = Ypos;
			GII->ForgetLastMouseCoords = false;
			return;
		}

		// calculates offsets and updates last x and y pos
		float Xoffset = Xpos - GII->MouseCoords.LastX;
		float Yoffset = GII->MouseCoords.LastY - Ypos;
		GII->MouseCoords.LastX = Xpos;
		GII->MouseCoords.LastY = Ypos;

		auto* CamManager = RCameraManager::Get();
		Xoffset *= CamManager->GetCurrentCamera()->Sensitivity;
		Yoffset *= CamManager->GetCurrentCamera()->Sensitivity;

		CamManager->ChangeCameraDirection(CamManager->GetCurrentCamera(), Xoffset, Yoffset);
	}

	// updates mouse position
	GII->MouseCoords.X = Xpos;
	GII->MouseCoords.Y = Ypos;
}


void OnMouseScroll(GLFWwindow* Window, double Xoffset, double Yoffset)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	auto* CamManager = RCameraManager::Get();
	auto* Camera = CamManager->GetCurrentCamera();
	Camera->Position += static_cast<float>(3 * Yoffset) * Camera->Front;
}


void OnMouseBtn(GLFWwindow* Window, int Button, int Action, int Mods)
{
	auto* GII = GlobalInputInfo::Get();

	// @todo: need to refactor registering mouse click into the KeyInput struct and having it
	// acknowledge whether you are clicking or dragging or what
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	switch (Button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:
		{
			if (Action == GLFW_PRESS)
			{
				GII->MouseState |= MouseLbClick;
			}
			else if (Action == GLFW_RELEASE)
			{
				GII->MouseState &= ~(MouseLbClick);
				GII->MouseState &= ~(MouseLbHold);
				GII->MouseState &= ~(MouseLbDragging);
			}
			break;
		}
		case GLFW_MOUSE_BUTTON_RIGHT:
		{
			if (Action == GLFW_PRESS)
			{
				GII->MouseState |= MouseRbClick;
				GII->ForgetLastMouseCoords = true;
				GII->MouseCoords.ClickX = GII->MouseCoords.X;
				GII->MouseCoords.ClickY = GII->MouseCoords.Y;
			}
			else if (Action == GLFW_RELEASE)
			{
				GII->MouseState &= ~(MouseRbClick);
				GII->MouseState &= ~(MouseRbHold);
				GII->MouseState &= ~(MouseRbDragging);
			}
			break;
		}
		default:
			break;
	}
}


bool PressedOnce(RInputFlags Flags, uint64 Key)
{
	auto* GII = GlobalInputInfo::Get();
	return Flags.KeyPress & Key && !(GII->KeyState & Key);
}


bool PressedOnly(RInputFlags Flags, uint64 Key)
{
	auto* GII = GlobalInputInfo::Get();
	return Flags.KeyPress == Key && !(GII->KeyState & Key);
}


bool Pressed(RInputFlags Flags, uint64 Key)
{
	return Flags.KeyPress & Key;
}


void CheckMouseClickHold()
{
	auto* GII = GlobalInputInfo::Get();
	if ((GII->MouseState & MouseLbClick))
	{
		GII->MouseState &= ~(MouseLbClick);
		GII->MouseState |= MouseLbHold;
	}
	if ((GII->MouseState & MouseRbClick))
	{
		GII->MouseState &= ~(MouseRbClick);
		GII->MouseState |= MouseRbHold;
	}
}


void ResetInputFlags(RInputFlags Flags)
{
	auto* GII = GlobalInputInfo::Get();
	// here we record a history for if keys were last pressed or released, so to enable smooth toggle
	GII->KeyState |= Flags.KeyPress;
	GII->KeyState &= ~Flags.KeyRelease;
}
