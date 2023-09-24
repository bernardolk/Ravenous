#pragma once

#include "engine/core/core.h"
#include "primitives/ray.h"
#include "engine/geometry/triangle.h"

struct EEntity;
struct RRaycastTest
{
	bool hit = false;
	float distance = 0;
	EEntity* entity = nullptr;
	int obj_hit_index = -1;
	std::string obj_hit_type{};
	RRay ray{};
	RTriangle t{};
	u16 t_index = 0;
};

enum NRayCastType
{
	RayCast_TestOnlyFromOutsideIn   = 0,
	RayCast_TestBothSidesOfTriangle = 1,
	RayCast_TestOnlyVisibleEntities = 2
};

RRay CastPickray(RCamera* camera, double screen_x, double screen_y);
RRaycastTest CL_TestAgainstRay(RRay ray, EEntity* entity, NRayCastType test_type, float max_distance);
RRaycastTest CL_TestAgainstRay(RRay ray, EEntity* entity);
RRaycastTest CL_TestAgainstRay(RRay ray, RMesh* mesh, glm::mat4 mat_model, NRayCastType test_type);
RRaycastTest CL_TestAgainstRay(RRay ray, RTriangle triangle, bool test_both_sides = true);
RRaycastTest CL_TestAgainstRay(RRay ray, RCollisionMesh* collider, NRayCastType test_type);
vec3 CL_GetPointFromDetection(RRay ray, RRaycastTest result);
bool CL_TestAgainstRay(RRay ray, RBoundingBox box);
