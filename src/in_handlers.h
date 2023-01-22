#pragma once

#include "engine/core/core.h"

void IN_handle_common_input(InputFlags flags, Player* & player);
void IN_handle_movement_input(InputFlags flags, Player* & player, World* world);

u64 KEY_MOVE_UP, KEY_MOVE_DOWN, KEY_MOVE_LEFT, KEY_MOVE_RIGHT, KEY_DASH, KEY_WALK, KEY_ACTION;


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
	if(pressed(flags, KEY_MOVE_UP))
	{
		v_dir += nrmlz(to_xz(PCam->front));
	}
	if(pressed(flags, KEY_MOVE_LEFT))
	{
		vec3 onwards_vector = cross(PCam->front, PCam->up);
		v_dir -= nrmlz(to_xz(onwards_vector));
	}
	if(pressed(flags, KEY_MOVE_DOWN))
	{
		v_dir -= nrmlz(to_xz(PCam->front));
	}
	if(pressed(flags, KEY_MOVE_RIGHT))
	{
		vec3 onwards_vector = cross(PCam->front, PCam->up);
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
			if(pressed(flags, KEY_LEFT_SHIFT) && GlobalInputInfo::Get()->mouse_state & MOUSE_LB_CLICK)
				player->want_to_grab = true;

			// INTERACT
			if(pressed_once(flags, KEY_ACTION))
			{
				GP_check_trigger_interaction(player, world);
				player->dodge_btn = true;
			}

			break;
		}

		case PLAYER_STATE_JUMPING:
		{
			// MID-AIR CONTROL IF JUMPING UP
			if(player->jumping_upwards)
				IN_process_move_keys(flags, v_dir);

			if(pressed(flags, KEY_DASH))
				player->action = true;

			break;
		}

		case PLAYER_STATE_FALLING:
		{
			if(pressed(flags, KEY_DASH))
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
			if(pressed(flags, KEY_DASH))
			{
				player->action = true;

				if(pressed(flags, KEY_MOVE_UP))
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
	if(pressed_once(flags, KEY_COMMA))
	{
		if(Rvn::frame.time_step > 0)
		{
			Rvn::frame.time_step -= 0.025;
		}
	}
	if(pressed_once(flags, KEY_PERIOD))
	{
		if(Rvn::frame.time_step < 3)
		{
			Rvn::frame.time_step += 0.025;
		}
	}
	if(pressed_once(flags, KEY_1))
	{
		Rvn::rm_buffer->Add("TIME STEP x0.05", 1000);
		Rvn::frame.time_step = 0.05;
	}
	if(pressed_once(flags, KEY_2))
	{
		Rvn::rm_buffer->Add("TIME STEP x0.1", 1000);
		Rvn::frame.time_step = 0.1;
	}
	if(pressed_once(flags, KEY_3))
	{
		Rvn::rm_buffer->Add("TIME STEP x0.3", 1000);
		Rvn::frame.time_step = 0.3;
	}
	if(pressed_once(flags, KEY_4))
	{
		Rvn::rm_buffer->Add("TIME STEP x1.0", 1000);
		Rvn::frame.time_step = 1.0;
	}
	if(pressed_once(flags, KEY_5))
	{
		Rvn::rm_buffer->Add("TIME STEP x2.0", 1000);
		Rvn::frame.time_step = 2.0;
	}
	if(flags.key_press & KEY_K)
	{
		player->Die();
	}
	if(pressed_once(flags, KEY_F))
	{
		EngineState::ToggleProgramMode();
	}
	if(pressed_once(flags, KEY_GRAVE_TICK))
	{
		start_console_mode();
	}
	if(flags.key_press & KEY_ESC && flags.key_press & KEY_LEFT_SHIFT)
	{
		auto* GDC = GlobalDisplayConfig::Get();
		glfwSetWindowShouldClose(GDC->window, true);
	}
	if(pressed_once(flags, KEY_Y))
	{
		// for testing EPA collision resolve
		GlobalSceneInfo::Get()->tmp_unstuck_things = true;
	}
}
