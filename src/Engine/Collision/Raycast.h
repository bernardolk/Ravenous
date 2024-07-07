#pragma once

#include "engine/core/core.h"
#include "primitives/ray.h"
#include "engine/geometry/triangle.h"

struct EEntity;
struct RRaycastTest
{
	bool Hit = false;
	float Distance = -1;
	//@entityptr
	EEntity* Entity = nullptr;
	RRay Ray;
	RTriangle Triangle;
	
	vec3 GetPoint() { return Ray.Origin + Ray.Direction * Distance; }
};

//@TODO: These should be refactored as flags 
enum NRayCastType
{
	RayCast_TestOnlyFromOutsideIn   = 0,
	RayCast_TestBothSidesOfTriangle = 1,
	RayCast_TestOnlyVisibleEntities = 2
};

RRay CastPickray(RCamera* Camera = nullptr);
RRay CastFirstPersonRay();

RRaycastTest TestRayAgainstEntity(const RRay& Ray, EEntity* Entity, NRayCastType TestType = RayCast_TestOnlyVisibleEntities);
RRaycastTest TestRayAgainstMesh(const RRay& Ray, RMesh* Mesh, mat4 MatModel, NRayCastType TestType);
RRaycastTest TestRayAgainstTriangle(const RRay& Ray, RTriangle Triangle, bool TestBothSides = true);
RRaycastTest TestRayAgainstQuad(const RRay& Ray, const RQuad& Quad, bool TestBothSides = true);
RRaycastTest TestRayAgainstCollider(const RRay& Ray, RCollisionMesh* Collider, NRayCastType TestType);
bool TestRayAgainstBoundingBox(const RRay& Ray, RBoundingBox Box);
