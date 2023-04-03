#pragma once

#include "engine/core/core.h"
#include "primitives/ray.h"
#include "engine/geometry/triangle.h"

struct E_Entity;
struct RaycastTest
{
	bool hit = false;
	float distance = 0;
	E_Entity* entity = nullptr;
	int obj_hit_index = -1;
	std::string obj_hit_type{};
	Ray ray{};
	Triangle t{};
	u16 t_index = 0;
};

enum RayCastType
{
	RayCast_TestOnlyFromOutsideIn   = 0,
	RayCast_TestBothSidesOfTriangle = 1,
	RayCast_TestOnlyVisibleEntities = 2
};

Ray CastPickray(Camera* camera, double screen_x, double screen_y);
RaycastTest CL_TestAgainstRay(Ray ray, E_Entity* entity, RayCastType test_type, float max_distance);
RaycastTest CL_TestAgainstRay(Ray ray, E_Entity* entity);
RaycastTest CL_TestAgainstRay(Ray ray, Mesh* mesh, glm::mat4 mat_model, RayCastType test_type);
RaycastTest CL_TestAgainstRay(Ray ray, Triangle triangle, bool test_both_sides = true);
RaycastTest CL_TestAgainstRay(Ray ray, CollisionMesh* collider, RayCastType test_type);
vec3 CL_GetPointFromDetection(Ray ray, RaycastTest result);
bool CL_TestAgainstRay(Ray ray, BoundingBox box);
