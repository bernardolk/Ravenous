#include "InputPhase.h"
#include "engine/io/display.h"
#include "engine/io/input.h"
#include "editor/EditorState.h"
#include <glfw3.h>
#include <imgui.h>
#include "engine/camera/camera.h"


RInputFlags StartInputPhase()
{
	auto* GII = GlobalInputInfo::Get();
	GII->MouseCoords.LastX = GII->MouseCoords.X;
	GII->MouseCoords.LastY = GII->MouseCoords.Y;
	
	CheckMouseClickHold();
	glfwPollEvents();
	
	auto* GDC = GlobalDisplayState::Get();
	auto KeyPressFlags = ProcessKeyboardInputKeyPress(GDC->GetWindow());
	auto KeyReleaseFlags = ProcessKeyboardInputKeyRelease(GDC->GetWindow());
	return RInputFlags{KeyPressFlags, KeyReleaseFlags};
}


uint64 ProcessKeyboardInputKeyPress(GLFWwindow* Window)
{
	uint64 Flags = 0;

	if (glfwGetKey(Window, GLFW_KEY_Q) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyQ;

	if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyW;

	if (glfwGetKey(Window, GLFW_KEY_E) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyE;

	if (glfwGetKey(Window, GLFW_KEY_R) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyR;

	if (glfwGetKey(Window, GLFW_KEY_T) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyT;

	if (glfwGetKey(Window, GLFW_KEY_Y) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyY;

	if (glfwGetKey(Window, GLFW_KEY_U) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyU;

	if (glfwGetKey(Window, GLFW_KEY_I) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyI;

	if (glfwGetKey(Window, GLFW_KEY_O) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyO;

	if (glfwGetKey(Window, GLFW_KEY_P) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyP;

	if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyA;

	if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyS;

	if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyD;

	if (glfwGetKey(Window, GLFW_KEY_F) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyF;

	if (glfwGetKey(Window, GLFW_KEY_G) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyG;

	if (glfwGetKey(Window, GLFW_KEY_H) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyH;

	if (glfwGetKey(Window, GLFW_KEY_J) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyJ;

	if (glfwGetKey(Window, GLFW_KEY_K) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyK;

	if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyL;

	if (glfwGetKey(Window, GLFW_KEY_Z) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyZ;

	if (glfwGetKey(Window, GLFW_KEY_X) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyX;

	if (glfwGetKey(Window, GLFW_KEY_C) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyC;

	if (glfwGetKey(Window, GLFW_KEY_V) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyV;

	if (glfwGetKey(Window, GLFW_KEY_B) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyB;

	if (glfwGetKey(Window, GLFW_KEY_N) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyN;

	if (glfwGetKey(Window, GLFW_KEY_M) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyM;

	if (glfwGetKey(Window, GLFW_KEY_0) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key0;

	if (glfwGetKey(Window, GLFW_KEY_1) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key1;

	if (glfwGetKey(Window, GLFW_KEY_2) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key2;

	if (glfwGetKey(Window, GLFW_KEY_3) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key3;

	if (glfwGetKey(Window, GLFW_KEY_4) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key4;

	if (glfwGetKey(Window, GLFW_KEY_5) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key5;

	if (glfwGetKey(Window, GLFW_KEY_6) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key6;

	if (glfwGetKey(Window, GLFW_KEY_7) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key7;

	if (glfwGetKey(Window, GLFW_KEY_8) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key8;

	if (glfwGetKey(Window, GLFW_KEY_9) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::Key9;

	if (glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyLeftShift;

	if (glfwGetKey(Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyLeftCtrl;

	if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyEsc;

	if (glfwGetKey(Window, GLFW_KEY_UP) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyUp;

	if (glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyDown;

	if (glfwGetKey(Window, GLFW_KEY_LEFT) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyLeft;

	if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyRight;

	if (glfwGetKey(Window, GLFW_KEY_SPACE) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeySpace;

	if (glfwGetKey(Window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyGraveTick;

	if (glfwGetKey(Window, GLFW_KEY_ENTER) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyEnter;

	if (glfwGetKey(Window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyBackspace;

	if (glfwGetKey(Window, GLFW_KEY_COMMA) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyComma;

	if (glfwGetKey(Window, GLFW_KEY_PERIOD) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyPeriod;

	if (glfwGetKey(Window, GLFW_KEY_DELETE) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyDelete;

	if (glfwGetKey(Window, GLFW_KEY_PAUSE) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyPause;

	if (glfwGetKey(Window, GLFW_KEY_END) == GLFW_PRESS)
		Flags = Flags | (uint64)NKeyInput::KeyEnd;
	
	return Flags;
}


uint64 ProcessKeyboardInputKeyRelease(GLFWwindow* Window)
{
	uint64 Flags = 0;

	if (glfwGetKey(Window, GLFW_KEY_Q) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyQ;

	if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyW;

	if (glfwGetKey(Window, GLFW_KEY_E) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyE;

	if (glfwGetKey(Window, GLFW_KEY_R) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyR;

	if (glfwGetKey(Window, GLFW_KEY_T) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyT;

	if (glfwGetKey(Window, GLFW_KEY_Y) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyY;

	if (glfwGetKey(Window, GLFW_KEY_U) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyU;

	if (glfwGetKey(Window, GLFW_KEY_I) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyI;

	if (glfwGetKey(Window, GLFW_KEY_O) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyO;

	if (glfwGetKey(Window, GLFW_KEY_P) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyP;

	if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyA;

	if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyS;

	if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyD;

	if (glfwGetKey(Window, GLFW_KEY_F) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyF;

	if (glfwGetKey(Window, GLFW_KEY_G) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyG;

	if (glfwGetKey(Window, GLFW_KEY_H) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyH;

	if (glfwGetKey(Window, GLFW_KEY_J) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyJ;

	if (glfwGetKey(Window, GLFW_KEY_K) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyK;

	if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyL;

	if (glfwGetKey(Window, GLFW_KEY_Z) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyZ;

	if (glfwGetKey(Window, GLFW_KEY_X) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyX;

	if (glfwGetKey(Window, GLFW_KEY_C) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyC;

	if (glfwGetKey(Window, GLFW_KEY_V) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyV;

	if (glfwGetKey(Window, GLFW_KEY_B) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyB;

	if (glfwGetKey(Window, GLFW_KEY_N) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyN;

	if (glfwGetKey(Window, GLFW_KEY_M) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyM;

	if (glfwGetKey(Window, GLFW_KEY_0) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key0;

	if (glfwGetKey(Window, GLFW_KEY_1) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key1;

	if (glfwGetKey(Window, GLFW_KEY_2) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key2;

	if (glfwGetKey(Window, GLFW_KEY_3) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key3;

	if (glfwGetKey(Window, GLFW_KEY_4) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key4;

	if (glfwGetKey(Window, GLFW_KEY_5) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key5;

	if (glfwGetKey(Window, GLFW_KEY_6) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key6;

	if (glfwGetKey(Window, GLFW_KEY_7) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key7;

	if (glfwGetKey(Window, GLFW_KEY_8) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key8;

	if (glfwGetKey(Window, GLFW_KEY_9) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::Key9;

	if (glfwGetKey(Window, GLFW_KEY_UP) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyUp;

	if (glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyDown;

	if (glfwGetKey(Window, GLFW_KEY_LEFT) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyLeft;

	if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyRight;

	if (glfwGetKey(Window, GLFW_KEY_SPACE) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeySpace;

	if (glfwGetKey(Window, GLFW_KEY_GRAVE_ACCENT) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyGraveTick;

	if (glfwGetKey(Window, GLFW_KEY_ENTER) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyEnter;

	if (glfwGetKey(Window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyBackspace;

	if (glfwGetKey(Window, GLFW_KEY_COMMA) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyComma;

	if (glfwGetKey(Window, GLFW_KEY_PERIOD) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyPeriod;

	if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyEsc;

	if (glfwGetKey(Window, GLFW_KEY_DELETE) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyDelete;

	if (glfwGetKey(Window, GLFW_KEY_PAUSE) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyPause;
	
	if (glfwGetKey(Window, GLFW_KEY_END) == GLFW_RELEASE)
		Flags = Flags | (uint64)NKeyInput::KeyEnd;
	
	return Flags;
}


void OnMouseMove(GLFWwindow* Window, double Xpos, double Ypos)
{
	auto* GII = GlobalInputInfo::Get();
	auto* ES = REditorState::Get();

	if (REditorState::IsInEditorMode() && ImGui::GetIO().WantCaptureMouse)
		return;

	// activates mouse dragging if clicking and current mouse position has changed a certain ammount
	if (!(GII->MouseState & (uint64)NMouseInput::RightButtonDragging) && GII->MouseState & (uint64)NMouseInput::RightButtonHold)
	{
		auto OffsetFromClickX = abs(GII->MouseCoords.ClickX - GII->MouseCoords.X);
		auto OffsetFromClickY = abs(GII->MouseCoords.ClickY - GII->MouseCoords.Y);
		if (OffsetFromClickX > 2 || OffsetFromClickY > 2)
		{
			GII->MouseState |= (uint64)NMouseInput::RightButtonDragging;
		}
	}

	// do the same for LB
	if (!(GII->MouseState & (uint64)NMouseInput::LeftButtonDragging) && GII->MouseState & (uint64)NMouseInput::LeftButtonHold)
	{
		auto OffsetFromClickX = abs(GII->MouseCoords.ClickX - GII->MouseCoords.X);
		auto OffsetFromClickY = abs(GII->MouseCoords.ClickY - GII->MouseCoords.Y);
		if (OffsetFromClickX > 2 || OffsetFromClickY > 2)
		{
			GII->MouseState |= (uint64)NMouseInput::LeftButtonDragging;
		}
	}

	// @todo: should refactor this out of here
	// MOVE CAMERA WITH MOUSE IF APPROPRIATE
	if (ES->CurrentMode == REditorState::NProgramMode::Game || (GII->MouseState & (uint64)NMouseInput::RightButtonDragging))
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
		double Xoffset = Xpos - GII->MouseCoords.LastX;
		double Yoffset = GII->MouseCoords.LastY - Ypos;
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
				GII->MouseState |= (uint64)NMouseInput::LeftButtonClick;
			}
			else if (Action == GLFW_RELEASE)
			{
				GII->MouseState &= ~((uint64)NMouseInput::LeftButtonClick);
				GII->MouseState &= ~((uint64)NMouseInput::LeftButtonHold);
				GII->MouseState &= ~((uint64)NMouseInput::LeftButtonDragging);
			}
			break;
		}
		case GLFW_MOUSE_BUTTON_RIGHT:
		{
			if (Action == GLFW_PRESS)
			{
				GII->MouseState |= (uint64)NMouseInput::RightButtonClick;
				GII->ForgetLastMouseCoords = true;
				GII->MouseCoords.ClickX = GII->MouseCoords.X;
				GII->MouseCoords.ClickY = GII->MouseCoords.Y;
			}
			else if (Action == GLFW_RELEASE)
			{
				GII->MouseState &= ~((uint64)NMouseInput::RightButtonClick);
				GII->MouseState &= ~((uint64)NMouseInput::RightButtonHold);
				GII->MouseState &= ~((uint64)NMouseInput::RightButtonDragging);
			}
			break;
		}
		default:
			break;
	}
}


bool PressedOnce(RInputFlags Flags, NKeyInput Key)
{
	auto* GII = GlobalInputInfo::Get();
	return Flags.KeyPress & (uint64)Key && !(GII->KeyState & (uint64)Key);
}


bool PressedOnceExclusively(RInputFlags Flags, NKeyInput Key)
{
	auto* GII = GlobalInputInfo::Get();
	return Flags.KeyPress == (uint64)Key && !(GII->KeyState & (uint64)Key);
}

bool PressedExclusively(RInputFlags Flags, NKeyInput Key)
{
	auto* GII = GlobalInputInfo::Get();
	return Flags.KeyPress == (uint64)Key;
}

bool Pressed(RInputFlags Flags, NKeyInput Key)
{
	return Flags.KeyPress & (uint64)Key;
}


void CheckMouseClickHold()
{
	auto* GII = GlobalInputInfo::Get();
	if ((GII->MouseState & (uint16)NMouseInput::LeftButtonClick))
	{
		GII->MouseState &= ~((uint16)NMouseInput::LeftButtonClick);
		GII->MouseState |= (uint16)NMouseInput::LeftButtonHold;
	}
	if ((GII->MouseState & (uint16)NMouseInput::RightButtonClick))
	{
		GII->MouseState &= ~((uint16)NMouseInput::RightButtonClick);
		GII->MouseState |= (uint16)NMouseInput::RightButtonHold;
	}
}


void ResetInputFlags(RInputFlags Flags)
{
	auto* GII = GlobalInputInfo::Get();
	// here we record a history for if keys were last pressed or released, so to enable smooth toggle
	GII->KeyState |= Flags.KeyPress;
	GII->KeyState &= ~Flags.KeyRelease;
}
