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
	bool Empty = true;
	vec3 A;
	vec3 B;

	vec3 DetectionDirection; // The direction of the ray / rays that detected the ledge
	vec3 SurfacePoint;       // The point in the horizontal surface that proves this is an actual ledge
};

struct RRaycastTest;

RRaycastTest ClGetTopHitFromMultipleRaycasts(RRay FirstRay, int Qty, float Spacing, EPlayer* Player);
RLedge ClPerformLedgeDetection(EPlayer* Player, RWorld* World);
vec3 ClGetFinalPositionLedgeVaulting(EPlayer* Player, RLedge Ledge);
