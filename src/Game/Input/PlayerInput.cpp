#pragma once

#include "game/input/PlayerInput.h"
#include "editor/console/console.h"
#include "game/entities/EPlayer.h"
#include "engine/utils/utils.h"
#include "engine/camera/camera.h"
#include "editor/EditorState.h"
#include "engine/io/input.h"
#include "engine/io/InputPhase.h"

void AssignKeysToActions()
{
	if (REditorState::IsInEditorMode())
	{
		KEY_MOVE_UP = KEY_UP;
		KEY_MOVE_DOWN = KEY_DOWN;
		KEY_MOVE_LEFT = KEY_LEFT;
		KEY_MOVE_RIGHT = KEY_RIGHT;
		KEY_DASH = KEY_Z;
		KEY_WALK = KEY_X;
		KEY_ACTION = KEY_J;
	}
	else if (REditorState::IsInGameMode())
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


void InProcessMoveKeys(RInputFlags Flags, vec3& VDir, bool ShortCircuit)
{
	auto* PlayerCamera = RCameraManager::Get()->GetGameCamera();

	if (Pressed(Flags, KEY_MOVE_UP))
	{
		v_dir += normalize(ToXz(player_camera->front));
		if (ShortCircuit)
			return;
	}
	if (Pressed(Flags, KEY_MOVE_LEFT))
	{
		vec3 OnwardsVector = Cross(player_camera->front, player_camera->up);
		v_dir -= normalize(ToXz(onwards_vector));
		if (ShortCircuit)
			return;
	}
	if (Pressed(Flags, KEY_MOVE_DOWN))
	{
		v_dir -= normalize(ToXz(player_camera->front));
		if (ShortCircuit)
			return;
	}
	if (Pressed(Flags, KEY_MOVE_RIGHT))
	{
		vec3 OnwardsVector = Cross(player_camera->front, player_camera->up);
		v_dir += normalize(ToXz(onwards_vector));
		if (ShortCircuit)
			return;
	}
}


void HandleMovementInput(RInputFlags Flags, EPlayer* Player, RWorld* World)
{
	// assign keys
	AssignKeysToActions();

	// reset player flags
	Player->dodge_btn = false;
	Player->interact_btn = false;
	Player->dashing = false;
	Player->walking = false;
	Player->action = false;
	Player->want_to_grab = false;
	Player->pressing_forward_while_in_air = false;
	Player->pressing_left_while_in_air = false;
	Player->pressing_right_while_in_air = false;
	Player->pressing_backward_while_in_air = false;
	Player->pressing_forward_while_standing = false;
	Player->pressing_left_while_standing = false;
	Player->pressing_right_while_standing = false;
	Player->pressing_backward_while_standing = false;

	// reset player 
	Player->v_dir = vec3(0);

	// combines all key presses into one v direction
	switch (Player->player_state)
	{
		case NPlayerState::Standing:
		{
			Player->stopped_pressing_forward_while_in_air = false;

			// MOVE
			IN_ProcessMoveKeys(Flags, Player->v_dir, false);

			if (Flags.KeyPress & KEY_MOVE_UP)
			{
				Player->pressing_forward_while_standing = true;
			}
			if (Flags.KeyPress & KEY_MOVE_LEFT)
			{
				Player->pressing_left_while_standing = true;
			}
			if (Flags.KeyPress & KEY_MOVE_RIGHT)
			{
				Player->pressing_right_while_standing = true;
			}
			if (Flags.KeyPress & KEY_MOVE_DOWN)
			{
				Player->pressing_backward_while_standing = true;
			}

			// SET PRIMARY MOVEMENT DIRECTION (through pressed key)
			if (player->IsMovingThisFrame() && player->first_pressed_movement_key_while_standing == KEY_NONE)
			{
				if (Flags.KeyPress & KEY_MOVE_UP)
					Player->first_pressed_movement_key_while_standing = KEY_MOVE_UP;

				else if (Flags.KeyPress & KEY_MOVE_LEFT)
					Player->first_pressed_movement_key_while_standing = KEY_MOVE_LEFT;

				else if (Flags.KeyPress & KEY_MOVE_RIGHT)
					Player->first_pressed_movement_key_while_standing = KEY_MOVE_RIGHT;

				else if (Flags.KeyPress & KEY_MOVE_DOWN)
					Player->first_pressed_movement_key_while_standing = KEY_MOVE_DOWN;
			}
			else if (!player->IsMovingThisFrame())
				player->first_pressed_movement_key_while_standing = KEY_NONE;

			// DASH
			if (Flags.KeyPress & KEY_DASH)
				Player->dashing = true;

			// WALK
			if (Flags.KeyPress & KEY_WALK)
				Player->walking = true;

			// JUMP
			if (flags.key_press & KEY_SPACE)
				Player->ChangeStateTo(NPlayerState::Jumping);

			// VAULT
			if (Pressed(flags, KEY_LEFT_SHIFT) && MOUSE_LB_CLICK & GlobalInputInfo::Get()->MouseState)
				Player->want_to_grab = true;

			// INTERACT
			if (PressedOnce(Flags, KEY_ACTION))
			{
				// GP_CheckTriggerInteraction(player, world);
				Player->interact_btn = true;
				Player->dodge_btn = true;
			}

			break;
		}

		case NPlayerState::Jumping:
		{
			// MID-AIR CONTROL IF JUMPING UP
			// if (player->jumping_upwards)
			IN_ProcessMoveKeys(Flags, Player->v_dir, false);

			if (Flags.KeyPress & KEY_MOVE_UP)
				Player->pressing_forward_while_in_air = true;
			else
				Player->stopped_pressing_forward_while_in_air = true;

			if (Flags.KeyPress & KEY_MOVE_LEFT)
				Player->pressing_left_while_in_air = true;

			if (Flags.KeyPress & KEY_MOVE_RIGHT)
				Player->pressing_right_while_in_air = true;

			if (Flags.KeyPress & KEY_MOVE_DOWN)
				Player->pressing_backward_while_in_air = true;

			if (Pressed(Flags, KEY_DASH))
				Player->action = true;

			break;
		}

		case NPlayerState::Falling:
		{
			IN_ProcessMoveKeys(Flags, Player->v_dir, false);

			if (Flags.KeyPress & KEY_MOVE_UP)
				Player->pressing_forward_while_in_air = true;
			else
				Player->stopped_pressing_forward_while_in_air = true;

			if (Flags.KeyPress & KEY_MOVE_LEFT)
				Player->pressing_left_while_in_air = true;

			if (Flags.KeyPress & KEY_MOVE_RIGHT)
				Player->pressing_right_while_in_air = true;

			if (Flags.KeyPress & KEY_MOVE_DOWN)
				Player->pressing_backward_while_in_air = true;

			if (Pressed(Flags, KEY_DASH))
				Player->action = true;

			break;
		}

		case NPlayerState::Sliding:
		{
			Player->v_dir = Player->sliding_direction;

			if (Flags.KeyPress & KEY_MOVE_LEFT)
			{
				auto LeftDir = Cross(player->sliding_normal, player->sliding_direction);
				Player->v_dir += left_dir;
				Player->v_dir = normalize(Player->v_dir);

			}
			if (Flags.KeyPress & KEY_MOVE_RIGHT)
			{
				auto RightDir = Cross(player->sliding_direction, player->sliding_normal);
				Player->v_dir += right_dir;
				Player->v_dir = normalize(Player->v_dir);
			}
			if (flags.key_press & KEY_SPACE)
				Player->ChangeStateTo(NPlayerState::Jumping);

			break;
		}
		case NPlayerState::Grabbing:
		{
			if (Pressed(Flags, KEY_DASH))
			{
				Player->action = true;

				if (Pressed(Flags, KEY_MOVE_UP))
					Player->ChangeStateTo(NPlayerState::Vaulting);
			}

			break;
		}
	}

	// normalize v_dir
	Player->v_dir = Player->v_dir != vec3(0.f, 0.f, 0.f) ? normalize(Player->v_dir) : Player->v_dir;
}
