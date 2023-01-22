#pragma once

#include "engine/core/core.h"

void GP_update_player_state(Player* & player, World* world);
vec3 GP_player_standing_get_next_position(Player* player);
void GP_check_trigger_interaction(Player* player, World* world);
void GP_check_player_grabbed_ledge(Player* player, World* world);
bool GP_check_player_vaulting(Player* player);
bool GP_simulate_player_collision_in_falling_trajectory(Player* player, vec2 xz_velocity);
// bool GP_scan_for_terrain(vec3 center, float radius, vec2 orientation0, float angle, int subdivisions);
// bool GP_do_vtrace_for_terrain(vec3 vtrace_origin, float terrain_baseline_height, vec3 debug_color);

//todo Wtf why is this so loose here.
inline float SlopeMinAngle = 0.4;
