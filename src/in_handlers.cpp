#pragma once

#include "in_handlers.h"
#include <glfw3.h>
#include "console.h"
#include "player.h"
#include "utils.h"
#include "engine/camera.h"
#include "engine/engine_state.h"
#include "engine/rvn.h"
#include "engine/io/display.h"
#include "engine/io/input.h"
#include "engine/loop/input_phase.h"
#include "engine/world/scene_manager.h"
#include "game/gameplay/gp_player_state.h"
#include "game/gameplay/gp_update.h"

void IN_assign_keys_to_actions()
{
	if(EngineState::IsInEditorMode())
	{
		KEY_MOVE_UP = KEY_UP;
		KEY_MOVE_DOWN = KEY_DOWN;
		KEY_MOVE_LEFT = KEY_LEFT;
		KEY_MOVE_RIGHT = KEY_RIGHT;
		KEY_DASH = KEY_Z;
		KEY_WALK = KEY_X;
		KEY_ACTION = KEY_J;
	}
	else if(EngineState::IsInGameMode())
	{
		KEY_MOVE_UP = KEY_W;
		KEY_MOVE_DOWN = KEY_S;
		KEY_MOVE_LEFT = KEY_A;
		KEY_MOVE_RIGHT = KEY_D;
		KEY_DASH = KEY_LEFT_SHIFT;
		KEY_WALK = KEY_LEFT_CTRL;
		KEY_ACTION = KEY_E;
	}
}


void IN_process_move_keys(InputFlags flags, vec3& v_dir)
{
	auto* player_camera = GlobalSceneInfo::GetGameCam();

	if(Pressed(flags, KEY_MOVE_UP))
	{
		v_dir += nrmlz(to_xz(player_camera->front));
	}
	if(Pressed(flags, KEY_MOVE_LEFT))
	{
		vec3 onwards_vector = cross(player_camera->front, player_camera->up);
		v_dir -= nrmlz(to_xz(onwards_vector));
	}
	if(Pressed(flags, KEY_MOVE_DOWN))
	{
		v_dir -= nrmlz(to_xz(player_camera->front));
	}
	if(Pressed(flags, KEY_MOVE_RIGHT))
	{
		vec3 onwards_vector = cross(player_camera->front, player_camera->up);
		v_dir += nrmlz(to_xz(onwards_vector));
	}
}


void IN_handle_movement_input(InputFlags flags, Player* & player, World* world)
{
	player->dodge_btn = false;

	// assign keys
	IN_assign_keys_to_actions();

	// reset player movement intention state
	player->dashing = false;
	player->walking = false;
	player->action = false;
	player->want_to_grab = false;
	auto& v_dir = player->v_dir;
	v_dir = vec3(0);

	// combines all key presses into one v direction
	switch(player->player_state)
	{

		case PLAYER_STATE_STANDING:
		{
			// MOVE
			IN_process_move_keys(flags, v_dir);

			// DASH
			if(flags.key_press & KEY_DASH)
				player->dashing = true;

			// WALK
			if(flags.key_press & KEY_WALK)
				player->walking = true;

			// JUMP
			if(flags.key_press & KEY_SPACE)
				GP_change_player_state(player, PLAYER_STATE_JUMPING);

			// VAULT
			if(Pressed(flags, KEY_LEFT_SHIFT) && MOUSE_LB_CLICK & GlobalInputInfo::Get()->mouse_state)
				player->want_to_grab = true;

			// INTERACT
			if(PressedOnce(flags, KEY_ACTION))
			{
				GP_CheckTriggerInteraction(player, world);
				player->dodge_btn = true;
			}

			break;
		}

		case PLAYER_STATE_JUMPING:
		{
			// MID-AIR CONTROL IF JUMPING UP
			if(player->jumping_upwards)
				IN_process_move_keys(flags, v_dir);

			if(Pressed(flags, KEY_DASH))
				player->action = true;

			break;
		}

		case PLAYER_STATE_FALLING:
		{
			if(Pressed(flags, KEY_DASH))
				player->action = true;

			break;
		}

		case PLAYER_STATE_SLIDING:
		{
			player->v_dir = player->sliding_direction;

			if(flags.key_press & KEY_MOVE_LEFT)
			{
				auto left_dir = cross(player->sliding_normal, player->sliding_direction);
				player->v_dir += left_dir;
				player->v_dir = normalize(player->v_dir);

			}
			if(flags.key_press & KEY_MOVE_RIGHT)
			{
				auto right_dir = cross(player->sliding_direction, player->sliding_normal);
				player->v_dir += right_dir;
				player->v_dir = normalize(player->v_dir);
			}
			if(flags.key_press & KEY_SPACE)
				GP_change_player_state(player, PLAYER_STATE_JUMPING);

			break;
		}
		case PLAYER_STATE_GRABBING:
		{
			if(Pressed(flags, KEY_DASH))
			{
				player->action = true;

				if(Pressed(flags, KEY_MOVE_UP))
					GP_change_player_state(player, PLAYER_STATE_VAULTING);
			}

			break;
		}
	}

	if(!(v_dir.x == 0.f && v_dir.y == 0.f && v_dir.z == 0.f))
		v_dir = normalize(v_dir);
}


// --------------
// SYSTEMS INPUT
// --------------
void IN_handle_common_input(InputFlags flags, Player* & player)
{
	if(PressedOnce(flags, KEY_COMMA))
	{
		if(Rvn::frame.time_step > 0)
		{
			Rvn::frame.time_step -= 0.025;
		}
	}
	if(PressedOnce(flags, KEY_PERIOD))
	{
		if(Rvn::frame.time_step < 3)
		{
			Rvn::frame.time_step += 0.025;
		}
	}
	if(PressedOnce(flags, KEY_1))
	{
		Rvn::rm_buffer->Add("TIME STEP x0.05", 1000);
		Rvn::frame.time_step = 0.05;
	}
	if(PressedOnce(flags, KEY_2))
	{
		Rvn::rm_buffer->Add("TIME STEP x0.1", 1000);
		Rvn::frame.time_step = 0.1;
	}
	if(PressedOnce(flags, KEY_3))
	{
		Rvn::rm_buffer->Add("TIME STEP x0.3", 1000);
		Rvn::frame.time_step = 0.3;
	}
	if(PressedOnce(flags, KEY_4))
	{
		Rvn::rm_buffer->Add("TIME STEP x1.0", 1000);
		Rvn::frame.time_step = 1.0;
	}
	if(PressedOnce(flags, KEY_5))
	{
		Rvn::rm_buffer->Add("TIME STEP x2.0", 1000);
		Rvn::frame.time_step = 2.0;
	}
	if(flags.key_press & KEY_K)
	{
		player->Die();
	}
	if(PressedOnce(flags, KEY_F))
	{
		EngineState::ToggleProgramMode();
	}
	if(PressedOnce(flags, KEY_GRAVE_TICK))
	{
		start_console_mode();
	}
	if(flags.key_press & KEY_ESC && flags.key_press & KEY_LEFT_SHIFT)
	{
		auto* GDC = GlobalDisplayConfig::Get();
		glfwSetWindowShouldClose(GDC->window, true);
	}
	if(PressedOnce(flags, KEY_Y))
	{
		// for testing EPA collision resolve
		GlobalSceneInfo::Get()->tmp_unstuck_things = true;
	}
}
