#pragma once
#include "engine/core/core.h"
#include "cl_resolvers.h"

struct ClResults;
struct ClResultsArray;
struct EntityBufferElement;

ClResultsArray CL_TestAndResolveCollisions(Player* player);

ClResults CL_TestCollisionBufferEntitites(
	Player* player,
	EntityBufferElement* entity_iterator,
	int entity_list_size,
	bool iterative
);

ClResults CL_TestPlayerVsEntity(Entity* entity, Player* player);
void CL_ResolveCollision(ClResults results, Player* player);
bool CL_TestCollisions(Player* player);
void CL_ResetCollisionBufferChecks();
void CL_RecomputeCollisionBufferEntities(Player* player);
bool CL_UpdatePlayerWorldCells(Player* player, World* world);
ClVtraceResult CL_DoStepoverVtrace(Player* player, World* world);
