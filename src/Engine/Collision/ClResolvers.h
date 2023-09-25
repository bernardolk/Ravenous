#pragma once

#include "engine/core/core.h"

struct RCollisionResults;

struct ClVtraceResult
{
	bool Hit = false;
	float DeltaY;
	EEntity* Entity;
};

void ClResolveCollision(RCollisionResults Results, EPlayer* Player);
void ClWallSlidePlayer(EPlayer* Player, vec3 WallNormal);
bool GpSimulatePlayerCollisionInFallingTrajectory(EPlayer* Player, vec2 XzVelocity);
bool ClRunTestsForFallSimulation(EPlayer* Player);
void ClMarkEntityChecked(const EEntity* Entity);


// fwd decl.
void GpUpdatePlayerState();
RCollisionResults CLTestPlayerVsEntity(EEntity* Entity, EPlayer* Player);

constexpr static float PlayerStepoverLimit = 0.21;
