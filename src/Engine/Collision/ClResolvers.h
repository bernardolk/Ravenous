#pragma once

#include "engine/core/core.h"

struct RCollisionResults;

struct ClVtraceResult
{
	bool hit = false;
	float delta_y;
	EEntity* entity;
};

void CL_ResolveCollision(RCollisionResults results, EPlayer* player);
void CL_WallSlidePlayer(EPlayer* player, vec3 wall_normal);
bool GP_SimulatePlayerCollisionInFallingTrajectory(EPlayer* player, vec2 xz_velocity);
bool CL_RunTestsForFallSimulation(EPlayer* player);
void CL_MarkEntityChecked(const EEntity* entity);


// fwd decl.
void GP_UpdatePlayerState();
RCollisionResults CLTestPlayerVsEntity(EEntity* entity, EPlayer* player);

constexpr static float PlayerStepoverLimit = 0.21;
