#pragma once
#include "engine/core/core.h"
#include "cl_resolvers.h"

struct ClResults;
struct ClResultsArray;
struct EntityBufferElement;

ClResultsArray CL_test_and_resolve_collisions(Player* player);

ClResults CL_test_collision_buffer_entitites(
	Player* player,
	EntityBufferElement* entity_iterator,
	int entity_list_size,
	bool iterative
);

ClResults CL_test_player_vs_entity(Entity* entity, Player* player);
void CL_ResolveCollision(ClResults results, Player* player);
bool CL_test_collisions(Player* player);
void CL_reset_collision_buffer_checks();
void CL_recompute_collision_buffer_entities(Player* player);
bool CL_update_player_world_cells(Player* player, World* world);
ClVtraceResult CL_DoStepoverVtrace(Player* player, World* world);
