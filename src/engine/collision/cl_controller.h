#pragma once

struct Entity;
struct Player;
struct CL_Results;
struct CL_ResultsArray;
struct EntityBufferElement;

CL_ResultsArray CL_test_and_resolve_collisions(Player* player);
CL_Results CL_test_collision_buffer_entitites(Player* player,
                                              EntityBufferElement* entity_iterator,
                                              int entity_list_size,
                                              bool iterative);
CL_Results CL_test_player_vs_entity(Entity* entity, Player* player);
void CL_resolve_collision(CL_Results results, Player* player);
bool CL_test_collisions(Player* player);
void CL_reset_collision_buffer_checks();
void CL_recompute_collision_buffer_entities(Player* player);
bool CL_update_player_world_cells(Player* player, World* world);
CL_VtraceResult CL_do_stepover_vtrace(Player* player, World* world);
