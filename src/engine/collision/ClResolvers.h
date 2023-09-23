#pragma once

#include "engine/core/core.h"

struct ClResults;

struct ClVtraceResult
{
	bool hit = false;
	float delta_y;
	E_Entity* entity;
};

void CL_ResolveCollision(ClResults results, Player* player);
void CL_WallSlidePlayer(Player* player, vec3 wall_normal);
bool GP_SimulatePlayerCollisionInFallingTrajectory(Player* player, vec2 xz_velocity);
bool CL_RunTestsForFallSimulation(Player* player);
void CL_MarkEntityChecked(const E_Entity* entity);


// fwd decl.
void GP_UpdatePlayerState();
ClResults CLTestPlayerVsEntity(E_Entity* entity, Player* player);

constexpr static float PlayerStepoverLimit = 0.21;
