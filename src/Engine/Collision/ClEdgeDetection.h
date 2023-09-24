#pragma once

#include "engine/core/core.h"

// -------------------
// > LEDGE
// -------------------
// Definition: A geometric edge that the player can grab/use to stand on an horizontal surface
//    it must not be blocked above it. Meaning that the division between two stacked blocks, for instance, do not
//    contain a ledge (but it does contain an edge).

struct RLedge
{
	bool empty = true;
	vec3 a;
	vec3 b;

	vec3 detection_direction; // The direction of the ray / rays that detected the ledge
	vec3 surface_point;       // The point in the horizontal surface that proves this is an actual ledge
};

struct RRaycastTest;

RRaycastTest CL_GetTopHitFromMultipleRaycasts(RRay first_ray, int qty, float spacing, EPlayer* player);
RLedge CL_PerformLedgeDetection(EPlayer* player, RWorld* world);
vec3 CL_GetFinalPositionLedgeVaulting(EPlayer* player, RLedge ledge);
