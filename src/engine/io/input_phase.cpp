#include "input_phase.h"
#include "engine/io/display.h"
#include "engine/io/input.h"
#include "engine/engine_state.h"
#include <glfw3.h>
#include <imgui.h>
#include "engine/camera/camera.h"


InputFlags InputPhase()
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
	auto* GDC = GlobalDisplayConfig::Get();
	auto key_press_flags = ProcessKeyboardInputKeyPress(GDC->window);
	auto key_release_flags = ProcessKeyboardInputKeyRelease(GDC->window);
	return InputFlags{key_press_flags, key_release_flags};
}


u64 ProcessKeyboardInputKeyPress(GLFWwindow* window)
{
	u64 flags = 0;

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		flags = flags | KEY_Q;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		flags = flags | KEY_W;

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		flags = flags | KEY_E;

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		flags = flags | KEY_R;

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		flags = flags | KEY_T;

	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		flags = flags | KEY_Y;

	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		flags = flags | KEY_U;

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		flags = flags | KEY_I;

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		flags = flags | KEY_O;

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		flags = flags | KEY_P;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		flags = flags | KEY_A;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		flags = flags | KEY_S;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		flags = flags | KEY_D;

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		flags = flags | KEY_F;

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		flags = flags | KEY_G;

	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		flags = flags | KEY_H;

	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		flags = flags | KEY_J;

	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		flags = flags | KEY_K;

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		flags = flags | KEY_L;

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		flags = flags | KEY_Z;

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		flags = flags | KEY_X;

	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		flags = flags | KEY_C;

	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		flags = flags | KEY_V;

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		flags = flags | KEY_B;

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		flags = flags | KEY_N;

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		flags = flags | KEY_M;

	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
		flags = flags | KEY_0;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		flags = flags | KEY_1;

	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		flags = flags | KEY_2;

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		flags = flags | KEY_3;

	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		flags = flags | KEY_4;

	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		flags = flags | KEY_5;

	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		flags = flags | KEY_6;

	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
		flags = flags | KEY_7;

	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
		flags = flags | KEY_8;

	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
		flags = flags | KEY_9;

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		flags = flags | KEY_LEFT_SHIFT;

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		flags = flags | KEY_LEFT_CTRL;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		flags = flags | KEY_ESC;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		flags = flags | KEY_UP;

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		flags = flags | KEY_DOWN;

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		flags = flags | KEY_LEFT;

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		flags = flags | KEY_RIGHT;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		flags = flags | KEY_SPACE;

	if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
		flags = flags | KEY_GRAVE_TICK;

	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		flags = flags | KEY_ENTER;

	if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
		flags = flags | KEY_BACKSPACE;

	if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
		flags = flags | KEY_COMMA;

	if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
		flags = flags | KEY_PERIOD;

	if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS)
		flags = flags | KEY_DELETE;

	return flags;
}


u64 ProcessKeyboardInputKeyRelease(GLFWwindow* window)
{
	u64 flags = 0;

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
		flags = flags | KEY_Q;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
		flags = flags | KEY_W;

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
		flags = flags | KEY_E;

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
		flags = flags | KEY_R;

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
		flags = flags | KEY_T;

	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE)
		flags = flags | KEY_Y;

	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_RELEASE)
		flags = flags | KEY_U;

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE)
		flags = flags | KEY_I;

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE)
		flags = flags | KEY_O;

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
		flags = flags | KEY_P;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)
		flags = flags | KEY_A;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
		flags = flags | KEY_S;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
		flags = flags | KEY_D;

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
		flags = flags | KEY_F;

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
		flags = flags | KEY_G;

	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
		flags = flags | KEY_H;

	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE)
		flags = flags | KEY_J;

	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE)
		flags = flags | KEY_K;

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
		flags = flags | KEY_L;

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
		flags = flags | KEY_Z;

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE)
		flags = flags | KEY_X;

	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
		flags = flags | KEY_C;

	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE)
		flags = flags | KEY_V;

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
		flags = flags | KEY_B;

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE)
		flags = flags | KEY_N;

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
		flags = flags | KEY_M;

	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE)
		flags = flags | KEY_0;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
		flags = flags | KEY_1;

	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
		flags = flags | KEY_2;

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE)
		flags = flags | KEY_3;

	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE)
		flags = flags | KEY_4;

	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE)
		flags = flags | KEY_5;

	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE)
		flags = flags | KEY_6;

	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE)
		flags = flags | KEY_7;

	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE)
		flags = flags | KEY_8;

	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_RELEASE)
		flags = flags | KEY_9;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE)
		flags = flags | KEY_UP;

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE)
		flags = flags | KEY_DOWN;

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE)
		flags = flags | KEY_LEFT;

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
		flags = flags | KEY_RIGHT;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
		flags = flags | KEY_SPACE;

	if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_RELEASE)
		flags = flags | KEY_GRAVE_TICK;

	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
		flags = flags | KEY_ENTER;

	if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE)
		flags = flags | KEY_BACKSPACE;

	if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_RELEASE)
		flags = flags | KEY_COMMA;

	if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_RELEASE)
		flags = flags | KEY_PERIOD;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
		flags = flags | KEY_ESC;

	if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_RELEASE)
		flags = flags | KEY_DELETE;

	return flags;
}


void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
	auto* GII = GlobalInputInfo::Get();
	auto* ES = EngineState::Get();

	if (EngineState::IsInEditorMode() && ImGui::GetIO().WantCaptureMouse)
		return;

	// activates mouse dragging if clicking and current mouse position has changed a certain ammount
	if (!(GII->mouse_state & MOUSE_RB_DRAGGING) && GII->mouse_state & MOUSE_RB_HOLD)
	{
		auto offset_from_click_x = abs(GII->mouse_coords.click_x - GII->mouse_coords.x);
		auto offset_from_click_y = abs(GII->mouse_coords.click_y - GII->mouse_coords.y);
		if (offset_from_click_x > 2 || offset_from_click_y > 2)
		{
			GII->mouse_state |= MOUSE_RB_DRAGGING;
		}
	}

	// do the same for LB
	if (!(GII->mouse_state & MOUSE_LB_DRAGGING) && GII->mouse_state & MOUSE_LB_HOLD)
	{
		auto offset_from_click_x = abs(GII->mouse_coords.click_x - GII->mouse_coords.x);
		auto offset_from_click_y = abs(GII->mouse_coords.click_y - GII->mouse_coords.y);
		if (offset_from_click_x > 2 || offset_from_click_y > 2)
		{
			GII->mouse_state |= MOUSE_LB_DRAGGING;
		}
	}

	// @todo: should refactor this out of here
	// MOVE CAMERA WITH MOUSE IF APPROPRIATE
	if (ES->current_mode == EngineState::ProgramMode::Game || (GII->mouse_state & MOUSE_RB_DRAGGING))
	{
		if (GII->block_mouse_move)
			return;

		// 'teleports' stored coordinates to current mouse coordinates
		if (GII->forget_last_mouse_coords)
		{
			GII->mouse_coords.last_x = xpos;
			GII->mouse_coords.last_y = ypos;
			GII->forget_last_mouse_coords = false;
			return;
		}

		// calculates offsets and updates last x and y pos
		float xoffset = xpos - GII->mouse_coords.last_x;
		float yoffset = GII->mouse_coords.last_y - ypos;
		GII->mouse_coords.last_x = xpos;
		GII->mouse_coords.last_y = ypos;

		auto* cam_manager = CameraManager::Get();
		xoffset *= cam_manager->GetCurrentCamera()->sensitivity;
		yoffset *= cam_manager->GetCurrentCamera()->sensitivity;

		cam_manager->ChangeCameraDirection(cam_manager->GetCurrentCamera(), xoffset, yoffset);
	}

	// updates mouse position
	GII->mouse_coords.x = xpos;
	GII->mouse_coords.y = ypos;
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	auto* cam_manager = CameraManager::Get();
	auto* camera = cam_manager->GetCurrentCamera();
	camera->position += static_cast<float>(3 * yoffset) * camera->front;
}


void OnMouseBtn(GLFWwindow* window, int button, int action, int mods)
{
	auto* GII = GlobalInputInfo::Get();

	// @todo: need to refactor registering mouse click into the KeyInput struct and having it
	// acknowledge whether you are clicking or dragging or what
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	switch (button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:
		{
			if (action == GLFW_PRESS)
			{
				GII->mouse_state |= MOUSE_LB_CLICK;
			}
			else if (action == GLFW_RELEASE)
			{
				GII->mouse_state &= ~(MOUSE_LB_CLICK);
				GII->mouse_state &= ~(MOUSE_LB_HOLD);
				GII->mouse_state &= ~(MOUSE_LB_DRAGGING);
			}
			break;
		}
		case GLFW_MOUSE_BUTTON_RIGHT:
		{
			if (action == GLFW_PRESS)
			{
				GII->mouse_state |= MOUSE_RB_CLICK;
				GII->forget_last_mouse_coords = true;
				GII->mouse_coords.click_x = GII->mouse_coords.x;
				GII->mouse_coords.click_y = GII->mouse_coords.y;
			}
			else if (action == GLFW_RELEASE)
			{
				GII->mouse_state &= ~(MOUSE_RB_CLICK);
				GII->mouse_state &= ~(MOUSE_RB_HOLD);
				GII->mouse_state &= ~(MOUSE_RB_DRAGGING);
			}
			break;
		}
		default:
			break;
	}
}


bool PressedOnce(InputFlags flags, u64 key)
{
	auto* GII = GlobalInputInfo::Get();
	return flags.key_press & key && !(GII->key_state & key);
}


bool PressedOnly(InputFlags flags, u64 key)
{
	auto* GII = GlobalInputInfo::Get();
	return flags.key_press == key && !(GII->key_state & key);
}


bool Pressed(InputFlags flags, u64 key)
{
	return flags.key_press & key;
}


void CheckMouseClickHold()
{
	auto* GII = GlobalInputInfo::Get();
	if ((GII->mouse_state & MOUSE_LB_CLICK))
	{
		GII->mouse_state &= ~(MOUSE_LB_CLICK);
		GII->mouse_state |= MOUSE_LB_HOLD;
	}
	if ((GII->mouse_state & MOUSE_RB_CLICK))
	{
		GII->mouse_state &= ~(MOUSE_RB_CLICK);
		GII->mouse_state |= MOUSE_RB_HOLD;
	}
}


void ResetInputFlags(InputFlags flags)
{
	auto* GII = GlobalInputInfo::Get();
	// here we record a history for if keys were last pressed or released, so to enable smooth toggle
	GII->key_state |= flags.key_press;
	GII->key_state &= ~flags.key_release;
}
