#pragma once

#include "engine/core/core.h"
#include "primitives/ray.h"
#include "engine/geometry/triangle.h"

struct EEntity;
struct RRaycastTest
{
	bool Hit = false;
	float Distance = 0;
	EEntity* Entity = nullptr;
	int ObjHitIndex = -1;
	
	string ObjHitType;
	RRay Ray;
	RTriangle Triangle;
	uint16 TriangleIndex = 0;
};

enum NRayCastType
{
	RayCast_TestOnlyFromOutsideIn   = 0,
	RayCast_TestBothSidesOfTriangle = 1,
	RayCast_TestOnlyVisibleEntities = 2
};

RRay CastPickray(RCamera* Camera, double ScreenX, double ScreenY);
RRaycastTest ClTestAgainstRay(RRay Ray, EEntity* Entity, NRayCastType TestType, float MaxDistance);
RRaycastTest ClTestAgainstRay(RRay Ray, EEntity* Entity);
RRaycastTest ClTestAgainstRay(RRay Ray, RMesh* Mesh, glm::mat4 MatModel, NRayCastType TestType);
RRaycastTest ClTestAgainstRay(RRay Ray, RTriangle Triangle, bool TestBothSides = true);
RRaycastTest ClTestAgainstRay(RRay Ray, RCollisionMesh* Collider, NRayCastType TestType);
vec3 ClGetPointFromDetection(RRay Ray, RRaycastTest Result);
bool ClTestAgainstRay(RRay Ray, RBoundingBox Box);
