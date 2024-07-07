#pragma once

#include "game/input/PlayerInput.h"
#include "editor/console/console.h"
#include "..\Entities\Player.h"
#include "engine/utils/utils.h"
#include "engine/camera/camera.h"
#include "editor/EditorState.h"
#include "engine/io/input.h"
#include "engine/io/InputPhase.h"

void AssignKeysToActions()
{
	if (REditorState::IsInEditorMode())
	{
		RGameInputKey::MoveForward = (uint64) NKeyInput::KeyUp;
		RGameInputKey::MoveBackward = (uint64) NKeyInput::KeyDown;
		RGameInputKey::MoveLeft = (uint64) NKeyInput::KeyLeft;
		RGameInputKey::MoveRight = (uint64) NKeyInput::KeyRight;
		RGameInputKey::Dash = (uint64) NKeyInput::KeyZ;
		RGameInputKey::Walk = (uint64) NKeyInput::KeyX;
		RGameInputKey::Action = (uint64) NKeyInput::KeyJ;
	}
	else if (REditorState::IsInGameMode())
	{
		RGameInputKey::MoveForward = (uint64) NKeyInput::KeyW;
		RGameInputKey::MoveBackward = (uint64) NKeyInput::KeyS;
		RGameInputKey::MoveLeft = (uint64) NKeyInput::KeyA;
		RGameInputKey::MoveRight = (uint64) NKeyInput::KeyD;
		RGameInputKey::Dash = (uint64) NKeyInput::KeyLeftShift;
		RGameInputKey::Walk = (uint64) NKeyInput::KeyLeftCtrl;
		RGameInputKey::Action = (uint64) NKeyInput::KeyE;
	}
}


void InProcessMoveKeys(RInputFlags Flags, vec3& VDir, bool ShortCircuit)
{
	auto* PlayerCamera = RCameraManager::Get()->GetGameCamera();

	if (Pressed(Flags, RGameInputKey::MoveForward))
	{
		VDir += normalize(ToXZ(PlayerCamera->Front));
		if (ShortCircuit)
			return;
	}
	if (Pressed(Flags, RGameInputKey::MoveLeft))
	{
		vec3 OnwardsVector = Cross(PlayerCamera->Front, PlayerCamera->Up);
		VDir -= normalize(ToXZ(OnwardsVector));
		if (ShortCircuit)
			return;
	}
	if (Pressed(Flags, RGameInputKey::MoveBackward))
	{
		VDir -= normalize(ToXZ(PlayerCamera->Front));
		if (ShortCircuit)
			return;
	}
	if (Pressed(Flags, RGameInputKey::MoveRight))
	{
		vec3 OnwardsVector = Cross(PlayerCamera->Front, PlayerCamera->Up);
		VDir += normalize(ToXZ(OnwardsVector));
		if (ShortCircuit)
			return;
	}
}

// TODO: Address this, we shouldn't need something like this? 
bool Pressed(RInputFlags Flags, uint64 Key)
{
	return Pressed(Flags, (NKeyInput)Key);	
}

bool PressedOnly(RInputFlags Flags, uint64 Key)
{
	return PressedOnceExclusively(Flags, (NKeyInput)Key);	
}

bool PressedOnce(RInputFlags Flags, uint64 Key)
{
	return PressedOnce(Flags, (NKeyInput)Key);	
}

void InHandleMovementInput(RInputFlags Flags, EPlayer* Player, RWorld* World)
{
	// assign keys
	AssignKeysToActions();

	// reset player Flags
	Player->bDodgeButton = false;
	Player->bInteractButton = false;
	Player->bDashing = false;
	Player->bWalking = false;
	Player->bAction = false;
	Player->bWantToGrab = false;
	Player->bPressingForwardWhileInAir = false;
	Player->bPressingLeftWhileInAir = false;
	Player->bPressingRightWhileInAir = false;
	Player->bPressingBackwardWhileInAir = false;
	Player->bPressingForwardWhileStanding = false;
	Player->bPressingLeftWhileStanding = false;
	Player->bPressingRightWhileStanding = false;
	Player->bPressingBackwardWhileStanding = false;

	// reset player 
	Player->VDir = vec3(0);

	// combines all key presses into one v direction
	switch (Player->PlayerState)
	{
		case NPlayerState::Standing:
		{
			Player->bStoppedPressingForwardWhileInAir = false;

			// MOVE
			InProcessMoveKeys(Flags, Player->VDir, false);

			if (Flags.KeyPress & RGameInputKey::MoveForward)
			{
				Player->bPressingForwardWhileStanding = true;
			}
			if (Flags.KeyPress & RGameInputKey::MoveLeft)
			{
				Player->bPressingLeftWhileStanding = true;
			}
			if (Flags.KeyPress & RGameInputKey::MoveRight)
			{
				Player->bPressingRightWhileStanding = true;
			}
			if (Flags.KeyPress & RGameInputKey::MoveBackward)
			{
				Player->bPressingBackwardWhileStanding = true;
			}

			// SET PRIMARY MOVEMENT DIRECTION (through pressed key)
			if (Player->IsMovingThisFrame() && Player->FirstPressedMovementKeyWhileStanding == (uint64)NKeyInput::KeyNone)
			{
				if (Flags.KeyPress & RGameInputKey::MoveForward)
					Player->FirstPressedMovementKeyWhileStanding = RGameInputKey::MoveForward;

				else if (Flags.KeyPress & RGameInputKey::MoveLeft)
					Player->FirstPressedMovementKeyWhileStanding = RGameInputKey::MoveLeft;

				else if (Flags.KeyPress & RGameInputKey::MoveRight)
					Player->FirstPressedMovementKeyWhileStanding = RGameInputKey::MoveRight;

				else if (Flags.KeyPress & RGameInputKey::MoveBackward)
					Player->FirstPressedMovementKeyWhileStanding = RGameInputKey::MoveBackward;
			}
			else if (!Player->IsMovingThisFrame())
				Player->FirstPressedMovementKeyWhileStanding = (uint64)NKeyInput::KeyNone;

			// DASH
			if (Pressed(Flags, RGameInputKey::Dash))
				Player->bDashing = true;

			// WALK
			if (Pressed(Flags, RGameInputKey::Walk))
				Player->bWalking = true;

			// JUMP
			if (Pressed(Flags, NKeyInput::KeySpace))
				Player->ChangeStateTo(NPlayerState::Jumping);

			// VAULT
			if (Pressed(Flags, NKeyInput::KeyLeftShift) && (uint16)NMouseInput::LeftButtonClick & GlobalInputInfo::Get()->MouseState)
				Player->bWantToGrab = true;

			// INTERACT
			if (PressedOnce(Flags, RGameInputKey::Action))
			{
				// GP_CheckTriggerInteraction(player, world);
				Player->bInteractButton = true;
				Player->bDodgeButton = true;
			}

			break;
		}

		case NPlayerState::Jumping:
		{
			// MID-AIR CONTROL IF JUMPING UP
			// if (Player->jumping_Upwards)
			InProcessMoveKeys(Flags, Player->VDir, false);

			if (Flags.KeyPress & RGameInputKey::MoveForward)
				Player->bPressingForwardWhileInAir = true;
			else
				Player->bStoppedPressingForwardWhileInAir = true;

			if (Flags.KeyPress & RGameInputKey::MoveLeft)
				Player->bPressingLeftWhileInAir = true;

			if (Flags.KeyPress & RGameInputKey::MoveRight)
				Player->bPressingRightWhileInAir = true;

			if (Flags.KeyPress & RGameInputKey::MoveBackward)
				Player->bPressingBackwardWhileInAir = true;

			if (Pressed(Flags, RGameInputKey::Dash))
				Player->bAction = true;

			break;
		}

		case NPlayerState::Falling:
		{
			InProcessMoveKeys(Flags, Player->VDir, false);

			if (Flags.KeyPress & RGameInputKey::MoveForward)
				Player->bPressingForwardWhileInAir = true;
			else
				Player->bStoppedPressingForwardWhileInAir = true;

			if (Flags.KeyPress & RGameInputKey::MoveLeft)
				Player->bPressingLeftWhileInAir = true;

			if (Flags.KeyPress & RGameInputKey::MoveRight)
				Player->bPressingRightWhileInAir = true;

			if (Flags.KeyPress & RGameInputKey::MoveBackward)
				Player->bPressingBackwardWhileInAir = true;

			if (Pressed(Flags, RGameInputKey::Dash))
				Player->bAction = true;

			break;
		}

		case NPlayerState::Sliding:
		{
			Player->VDir = Player->SlidingDirection;

			if (Flags.KeyPress & RGameInputKey::MoveLeft)
			{
				auto LeftDir = Cross(Player->SlidingNormal, Player->SlidingDirection);
				Player->VDir += LeftDir;
				Player->VDir = normalize(Player->VDir);

			}
			if (Flags.KeyPress & RGameInputKey::MoveRight)
			{
				auto RightDir = Cross(Player->SlidingDirection, Player->SlidingNormal);
				Player->VDir += RightDir;
				Player->VDir = normalize(Player->VDir);
			}
			if (Flags.KeyPress & (uint64) NKeyInput::KeySpace)
				Player->ChangeStateTo(NPlayerState::Jumping);

			break;
		}
		case NPlayerState::Grabbing:
		{
			if (Pressed(Flags, RGameInputKey::Dash))
			{
				Player->bAction = true;

				if (Pressed(Flags, RGameInputKey::MoveForward))
					Player->ChangeStateTo(NPlayerState::Vaulting);
			}

			break;
		}
	}

	// normalize VDir
	Player->VDir = Player->VDir != vec3(0.f, 0.f, 0.f) ? normalize(Player->VDir) : Player->VDir;
}
