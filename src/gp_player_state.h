#pragma once

#include "player.h"
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
	Entity* entity = nullptr;
	vec3 normal = vec3(0);
	float penetration = 0;

	// grabbing info
	vec3 final_position = vec3(0);
	Ledge ledge;
};

void GP_player_state_change_jumping_to_falling(Player* player);
void GP_player_state_change_standing_to_falling(Player* player);
void GP_player_state_change_falling_to_standing(Player* player);
void GP_player_state_change_standing_to_jumping(Player* player);
void GP_player_state_change_any_to_sliding(Player* player, vec3 normal);
void GP_player_state_change_any_to_grabbing(Player* player, Entity* entity, vec2 normal_vec, vec3 final_position, float d);
void GP_player_state_change_grabbing_to_vaulting(Player* player);
void GP_player_state_change_standing_to_vaulting(Player* player, Ledge ledge, vec3 final_position);
void GP_player_state_change_vaulting_to_standing(Player* player);
void GP_player_state_change_standing_to_slide_falling(Player* player, Entity* ramp);
void GP_player_state_change_sliding_to_standing(Player* player);
void GP_player_state_change_sliding_to_jumping(Player* player);
void GP_player_state_change_sliding_to_falling(Player* player);

void GP_change_player_state(Player* player, PlayerState new_state, PlayerStateChangeArgs args = {});
