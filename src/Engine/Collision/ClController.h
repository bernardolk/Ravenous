#pragma once
#include "engine/core/core.h"
#include "ClResolvers.h"

struct ClResults;
struct EntityBufferElement;

Array<ClResults, 15> CL_TestAndResolveCollisions(Player* player);

ClResults CL_TestCollisionBufferEntitites(
	Player* player,
	bool iterative
);

ClResults CL_TestPlayerVsEntity(EEntity* entity, Player* player);
void CL_ResolveCollision(ClResults results, Player* player);
bool CL_TestCollisions(Player* player);
void CL_ResetCollisionBufferChecks();
void CL_RecomputeCollisionBufferEntities();
bool CL_UpdatePlayerWorldCells(Player* player);
ClVtraceResult CL_DoStepoverVtrace(Player* player, World* world);
