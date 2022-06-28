#pragma once

struct Entity;
struct Player;
struct CL_Results;
struct World;

struct CL_VtraceResult {
   bool     hit = false;
   float    delta_y;
   Entity*  entity;
};

void              CL_resolve_collision                                  (CL_Results results, Player* player);
void              CL_wall_slide_player                                  (Player* player, vec3 wall_normal);
bool              GP_simulate_player_collision_in_falling_trajectory    (Player* player, vec2 xz_velocity);
bool              CL_run_tests_for_fall_simulation                      (Player* player);
void              CL_mark_entity_checked                                (Entity* entity);


// fwd decl.
void        GP_update_player_state        (Player* &player);
CL_Results  CL_test_player_vs_entity      (Entity* entity, Player* player);

const static float PLAYER_STEPOVER_LIMIT = 0.21;