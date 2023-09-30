#pragma once
#include "engine/core/core.h"
#include "ClResolvers.h"

struct RCollisionResults;
struct EntityBufferElement;

Array<RCollisionResults, 15> ClTestAndResolveCollisions(EPlayer* Player);
RCollisionResults ClTestCollisionBufferEntitites(EPlayer* Player, bool Iterative);
RCollisionResults ClTestPlayerVsEntity(EEntity* Entity, EPlayer* Player);
void ResolveCollision(RCollisionResults Results, EPlayer* Player);
bool ClTestCollisions(EPlayer* Player);
void ClResetCollisionBufferChecks();
void ClRecomputeCollisionBufferEntities();
bool ClUpdatePlayerWorldCells(EPlayer* Player);
ClVtraceResult ClDoStepoverVtrace(EPlayer* Player, RWorld* World);
