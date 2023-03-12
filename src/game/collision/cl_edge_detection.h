#pragma once

#include "engine/core/core.h"

// -------------------
// > LEDGE
// -------------------
// Definition: A geometric edge that the player can grab/use to stand on an horizontal surface
//    it must not be blocked above it. Meaning that the division between two stacked blocks, for instance, do not
//    contain a ledge (but it does contain an edge).

struct Ledge
{
	bool empty = true;
	vec3 a;
	vec3 b;

	vec3 detection_direction; // The direction of the ray / rays that detected the ledge
	vec3 surface_point;       // The point in the horizontal surface that proves this is an actual ledge
};

struct RaycastTest;

RaycastTest CL_GetTopHitFromMultipleRaycasts(Ray first_ray, int qty, float spacing, Player* player);
Ledge CL_PerformLedgeDetection(Player* player, World* world);
vec3 CL_GetFinalPositionLedgeVaulting(Player* player, Ledge ledge);
