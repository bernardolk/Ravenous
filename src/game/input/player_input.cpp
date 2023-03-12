#pragma once

#include "game/input/player_input.h"
#include "editor/console/console.h"
#include "game/entities/player.h"
#include "engine/utils/utils.h"
#include "engine/camera/camera.h"
#include "engine/engine_state.h"
#include "engine/io/input.h"
#include "engine/io/input_phase.h"
#include "engine/world/scene_manager.h"
#include "game/gameplay/gp_player_state.h"
#include "game/gameplay/gp_update.h"

void IN_AssignKeysToActions()
{
	if (EngineState::IsInEditorMode())
	{
		KEY_MOVE_UP = KEY_UP;
		KEY_MOVE_DOWN = KEY_DOWN;
		KEY_MOVE_LEFT = KEY_LEFT;
		KEY_MOVE_RIGHT = KEY_RIGHT;
		KEY_DASH = KEY_Z;
		KEY_WALK = KEY_X;
		KEY_ACTION = KEY_J;
	}
	else if (EngineState::IsInGameMode())
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


void IN_ProcessMoveKeys(InputFlags flags, vec3& v_dir)
{
	auto* player_camera = GlobalSceneInfo::GetGameCam();

	if (Pressed(flags, KEY_MOVE_UP))
	{
		v_dir += normalize(ToXz(player_camera->front));
	}
	if (Pressed(flags, KEY_MOVE_LEFT))
	{
		vec3 onwards_vector = Cross(player_camera->front, player_camera->up);
		v_dir -= normalize(ToXz(onwards_vector));
	}
	if (Pressed(flags, KEY_MOVE_DOWN))
	{
		v_dir -= normalize(ToXz(player_camera->front));
	}
	if (Pressed(flags, KEY_MOVE_RIGHT))
	{
		vec3 onwards_vector = Cross(player_camera->front, player_camera->up);
		v_dir += normalize(ToXz(onwards_vector));
	}
}


void IN_HandleMovementInput(InputFlags flags, Player* & player, World* world)
{
	player->dodge_btn = false;

	// assign keys
	IN_AssignKeysToActions();

	// reset player movement intention state
	player->dashing = false;
	player->walking = false;
	player->action = false;
	player->want_to_grab = false;
	auto& v_dir = player->v_dir;
	v_dir = vec3(0);

	// combines all key presses into one v direction
	switch (player->player_state)
	{

		case PlayerState::Standing:
		{
			// MOVE
			IN_ProcessMoveKeys(flags, v_dir);

			// DASH
			if (flags.key_press & KEY_DASH)
				player->dashing = true;

			// WALK
			if (flags.key_press & KEY_WALK)
				player->walking = true;

			// JUMP
			if (flags.key_press & KEY_SPACE)
				GP_ChangePlayerState(player, PlayerState::Jumping);

			// VAULT
			if (Pressed(flags, KEY_LEFT_SHIFT) && MOUSE_LB_CLICK & GlobalInputInfo::Get()->mouse_state)
				player->want_to_grab = true;

			// INTERACT
			if (PressedOnce(flags, KEY_ACTION))
			{
				GP_CheckTriggerInteraction(player, world);
				player->dodge_btn = true;
			}

			break;
		}

		case PlayerState::Jumping:
		{
			// MID-AIR CONTROL IF JUMPING UP
			if (player->jumping_upwards)
				IN_ProcessMoveKeys(flags, v_dir);

			if (Pressed(flags, KEY_DASH))
				player->action = true;

			break;
		}

		case PlayerState::Falling:
		{
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
				GP_ChangePlayerState(player, PlayerState::Jumping);

			break;
		}
		case PlayerState::Grabbing:
		{
			if (Pressed(flags, KEY_DASH))
			{
				player->action = true;

				if (Pressed(flags, KEY_MOVE_UP))
					GP_ChangePlayerState(player, PlayerState::Vaulting);
			}

			break;
		}
	}

	if (!(v_dir.x == 0.f && v_dir.y == 0.f && v_dir.z == 0.f))
		v_dir = normalize(v_dir);
}
