#pragma once

#include "game/input/player_input.h"
#include "editor/console/console.h"
#include "game/entities/player.h"
#include "engine/utils/utils.h"
#include "engine/camera/camera.h"
#include "editor/editor_state.h"
#include "engine/io/input.h"
#include "engine/io/input_phase.h"

void IN_AssignKeysToActions()
{
	if (EditorState::IsInEditorMode())
	{
		KEY_MOVE_UP = KEY_UP;
		KEY_MOVE_DOWN = KEY_DOWN;
		KEY_MOVE_LEFT = KEY_LEFT;
		KEY_MOVE_RIGHT = KEY_RIGHT;
		KEY_DASH = KEY_Z;
		KEY_WALK = KEY_X;
		KEY_ACTION = KEY_J;
	}
	else if (EditorState::IsInGameMode())
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


void IN_ProcessMoveKeys(InputFlags flags, vec3& v_dir, bool short_circuit)
{
	auto* player_camera = CameraManager::Get()->GetGameCamera();

	if (Pressed(flags, KEY_MOVE_UP))
	{
		v_dir += normalize(ToXz(player_camera->front));
		if (short_circuit) return;
	}
	if (Pressed(flags, KEY_MOVE_LEFT))
	{
		vec3 onwards_vector = Cross(player_camera->front, player_camera->up);
		v_dir -= normalize(ToXz(onwards_vector));
		if (short_circuit) return;
	}
	if (Pressed(flags, KEY_MOVE_DOWN))
	{
		v_dir -= normalize(ToXz(player_camera->front));
		if (short_circuit) return;
	}
	if (Pressed(flags, KEY_MOVE_RIGHT))
	{
		vec3 onwards_vector = Cross(player_camera->front, player_camera->up);
		v_dir += normalize(ToXz(onwards_vector));
		if (short_circuit) return;
	}
}


void IN_HandleMovementInput(InputFlags flags, Player* player, World* world)
{
	// assign keys
	IN_AssignKeysToActions();

	// reset player flags
	player->dodge_btn = false;
	player->interact_btn = false;
	player->dashing = false;
	player->walking = false;
	player->action = false;
	player->want_to_grab = false;
	player->pressing_forward_while_in_air = false;
	player->pressing_left_while_in_air = false;
	player->pressing_right_while_in_air = false;
	player->pressing_backward_while_in_air = false;
	player->pressing_forward_while_standing = false;
	player->pressing_left_while_standing = false;
	player->pressing_right_while_standing = false;
	player->pressing_backward_while_standing = false;
	
	// reset player 
	player->v_dir = vec3(0);

	// combines all key presses into one v direction
	switch (player->player_state)
	{
		case PlayerState::Standing:
		{
			player->stopped_pressing_forward_while_in_air = false;
			
			// MOVE
			IN_ProcessMoveKeys(flags, player->v_dir, false);

			if (flags.key_press & KEY_MOVE_UP)
			{
				player->pressing_forward_while_standing = true;
			}
			if (flags.key_press & KEY_MOVE_LEFT)
			{
				player->pressing_left_while_standing = true;
			}
			if (flags.key_press & KEY_MOVE_RIGHT)
			{
				player->pressing_right_while_standing = true;
			}
			if (flags.key_press & KEY_MOVE_DOWN)
			{
				player->pressing_backward_while_standing = true;
			}

			// SET PRIMARY MOVEMENT DIRECTION (through pressed key)
			if (player->IsMovingThisFrame() && player->first_pressed_movement_key_while_standing == KEY_NONE)
			{
				if (flags.key_press & KEY_MOVE_UP)
					player->first_pressed_movement_key_while_standing = KEY_MOVE_UP;
				
				else if (flags.key_press & KEY_MOVE_LEFT)
					player->first_pressed_movement_key_while_standing = KEY_MOVE_LEFT;

				else if (flags.key_press & KEY_MOVE_RIGHT)
						player->first_pressed_movement_key_while_standing = KEY_MOVE_RIGHT;

				else if (flags.key_press & KEY_MOVE_DOWN)
						player->first_pressed_movement_key_while_standing = KEY_MOVE_DOWN;
			}
			else if (!player->IsMovingThisFrame())
				player->first_pressed_movement_key_while_standing = KEY_NONE;

			// DASH
			if (flags.key_press & KEY_DASH)
				player->dashing = true;

			// WALK
			if (flags.key_press & KEY_WALK)
				player->walking = true;

			// JUMP
			if (flags.key_press & KEY_SPACE)
				player->ChangeStateTo(PlayerState::Jumping);

			// VAULT
			if (Pressed(flags, KEY_LEFT_SHIFT) && MOUSE_LB_CLICK & GlobalInputInfo::Get()->mouse_state)
				player->want_to_grab = true;

			// INTERACT
			if (PressedOnce(flags, KEY_ACTION))
			{
				// GP_CheckTriggerInteraction(player, world);
				player->interact_btn = true;
				player->dodge_btn = true;
			}

			break;
		}

		case PlayerState::Jumping:
		{
			// MID-AIR CONTROL IF JUMPING UP
			// if (player->jumping_upwards)
			IN_ProcessMoveKeys(flags, player->v_dir, false);

			if (flags.key_press & KEY_MOVE_UP)
				player->pressing_forward_while_in_air = true;
			else
				player->stopped_pressing_forward_while_in_air = true;

			if (flags.key_press & KEY_MOVE_LEFT)
				player->pressing_left_while_in_air = true;
			
			if (flags.key_press & KEY_MOVE_RIGHT)
				player->pressing_right_while_in_air = true;

			if (flags.key_press & KEY_MOVE_DOWN)
				player->pressing_backward_while_in_air = true;
			
			if (Pressed(flags, KEY_DASH))
				player->action = true;

			break;
		}

		case PlayerState::Falling:
		{
			IN_ProcessMoveKeys(flags, player->v_dir, false);

			if (flags.key_press & KEY_MOVE_UP)
				player->pressing_forward_while_in_air = true;
			else
				player->stopped_pressing_forward_while_in_air = true;

			if (flags.key_press & KEY_MOVE_LEFT)
				player->pressing_left_while_in_air = true;
			
			if (flags.key_press & KEY_MOVE_RIGHT)
				player->pressing_right_while_in_air = true;

			if (flags.key_press & KEY_MOVE_DOWN)
				player->pressing_backward_while_in_air = true;

			if (Pressed(flags, KEY_DASH))
				player->action = true;

			break;
		}

		case PlayerState::Sliding:
		{
			player->v_dir = player->sliding_direction;

			if (flags.key_press & KEY_MOVE_LEFT)
			{
				auto left_dir = Cross(player->sliding_normal, player->sliding_direction);
				player->v_dir += left_dir;
				player->v_dir = normalize(player->v_dir);

			}
			if (flags.key_press & KEY_MOVE_RIGHT)
			{
				auto right_dir = Cross(player->sliding_direction, player->sliding_normal);
				player->v_dir += right_dir;
				player->v_dir = normalize(player->v_dir);
			}
			if (flags.key_press & KEY_SPACE)
				player->ChangeStateTo(PlayerState::Jumping);

			break;
		}
		case PlayerState::Grabbing:
		{
			if (Pressed(flags, KEY_DASH))
			{
				player->action = true;

				if (Pressed(flags, KEY_MOVE_UP))
					player->ChangeStateTo(PlayerState::Vaulting);
			}

			break;
		}
	}

	// normalize v_dir
	player->v_dir = player->v_dir != vec3(0.f, 0.f, 0.f) ? normalize(player->v_dir) : player->v_dir;
}
