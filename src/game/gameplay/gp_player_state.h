#pragma once

#include "game/entities/player.h"
#include "engine/core/core.h"
#include "game/collision/cl_edge_detection.h"

/* 
   --------------------
   > PLAYER STATE CODE
   --------------------
   Work in progress state machine-like modelling of player state 
*/

struct PlayerStateChangeArgs
{

	// collision
	E_Entity* entity = nullptr;
	vec3 normal = vec3(0);
	float penetration = 0;

	// grabbing info
	vec3 final_position = vec3(0);
	Ledge ledge;
};

void GP_PlayerStateChangeJumpingToFalling(Player* player);
void GP_PlayerStateChangeStandingToFalling(Player* player);
void GP_PlayerStateChangeFallingToStanding(Player* player);
void GP_PlayerStateChangeStandingToJumping(Player* player);
void GP_PlayerStateChangeAnyToSliding(Player* player, vec3 normal);
void GP_PlayerStateChangeAnyToGrabbing(Player* player, E_Entity* entity, vec2 normal_vec, vec3 final_position, float d);
void GP_PlayerStateChangeGrabbingToVaulting(Player* player);
void GP_PlayerStateChangeStandingToVaulting(Player* player, Ledge ledge, vec3 final_position);
void GP_PlayerStateChangeVaultingToStanding(Player* player);
void GP_PlayerStateChangeStandingToSlideFalling(Player* player, E_Entity* ramp);
void GP_PlayerStateChangeSlidingToStanding(Player* player);
void GP_PlayerStateChangeSlidingToJumping(Player* player);
void GP_PlayerStateChangeSlidingToFalling(Player* player);

void GP_ChangePlayerState(Player* player, PlayerState new_state, PlayerStateChangeArgs args = {});
