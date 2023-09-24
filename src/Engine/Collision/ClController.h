#pragma once
#include "engine/core/core.h"
#include "ClResolvers.h"

struct RCollisionResults;
struct EntityBufferElement;

Array<RCollisionResults, 15> CL_TestAndResolveCollisions(EPlayer* player);

RCollisionResults CL_TestCollisionBufferEntitites(
	EPlayer* player,
	bool iterative
);

RCollisionResults CL_TestPlayerVsEntity(EEntity* entity, EPlayer* player);
void CL_ResolveCollision(RCollisionResults results, EPlayer* player);
bool CL_TestCollisions(EPlayer* player);
void CL_ResetCollisionBufferChecks();
void CL_RecomputeCollisionBufferEntities();
bool CL_UpdatePlayerWorldCells(EPlayer* player);
ClVtraceResult CL_DoStepoverVtrace(EPlayer* player, RWorld* world);
