#include <engine/collision/raycast.h>
#include "engine/camera/camera.h"
#include <engine/collision/primitives/BoundingBox.h>
#include "engine/geometry/mesh.h"
#include <glm/gtx/normal.hpp>
#include <engine/geometry/triangle.h>
#include <engine/collision/primitives/ray.h>
#include <glm/gtx/quaternion.hpp>
#include <engine/collision/CollisionMesh.h>
#include "engine/entities/EEntity.h"
#include "engine/io/display.h"
#include "engine/utils/utils.h"

// --------------------------
// > TEST RAY AGAINST AABB
// --------------------------
bool CL_TestAgainstRay(Ray ray, BoundingBox box)
{
	vec3 ray_inv = ray.GetInv();

	float tx1 = (box.minx - ray.origin.x) * ray_inv.x;
	float tx2 = (box.maxx - ray.origin.x) * ray_inv.x;

	float tmin = Min(tx1, tx2);
	float tmax = Max(tx1, tx2);

	float ty1 = (box.miny - ray.origin.y) * ray_inv.y;
	float ty2 = (box.maxy - ray.origin.y) * ray_inv.y;

	tmin = Max(tmin, Min(ty1, ty2));
	tmax = Min(tmax, Max(ty1, ty2));

	float tz1 = (box.minz - ray.origin.z) * ray_inv.z;
	float tz2 = (box.maxz - ray.origin.z) * ray_inv.z;

	tmin = Max(tmin, Min(tz1, tz2));
	tmax = Min(tmax, Max(tz1, tz2));

	return tmax >= tmin;
}

// --------------------------
// > TEST RAY AGAINST ENTITY
// --------------------------
RaycastTest CL_TestAgainstRay(
	Ray ray, E_Entity* entity,
	RayCastType test_type = RayCast_TestOnlyFromOutsideIn,
	float max_distance = MaxFloat)
{
	// @TODO: when testing against player, we could:
	//      a) find the closest point between player's column and the ray
	//      b) do a sphere vs ray test 
	//      instead of testing the collider


	// first check collision with bounding box
	if (CL_TestAgainstRay(ray, entity->bounding_box))
	{
		//@TODO: We are not updating player's collider everytime now, so we must do it now on a raycast call
		entity->UpdateCollider();
		return CL_TestAgainstRay(ray, &entity->collider, test_type);
	}

	return RaycastTest{false};
}


RaycastTest CL_TestAgainstRay(Ray ray, E_Entity* entity)
{
	return CL_TestAgainstRay(ray, entity, RayCast_TestOnlyFromOutsideIn, MaxFloat);
}

// ---------------------------
// > TEST RAY AGAINT COLLIDER
// ---------------------------
// This doesn't take a matModel
RaycastTest CL_TestAgainstRay(Ray ray, CollisionMesh* collider, RayCastType test_type)
{

	int triangles = collider->indices.size() / 3;
	float min_distance = MaxFloat;
	RaycastTest min_hit_test{false, -1};
	for (int i = 0; i < triangles; i++)
	{
		Triangle t = get_triangle_for_collider_indexed_mesh(collider, i);
		bool test_both_sides = test_type == RayCast_TestBothSidesOfTriangle;
		auto test = CL_TestAgainstRay(ray, t, test_both_sides);
		if (test.hit && test.distance < min_distance)
		{
			min_hit_test = test;
			min_hit_test.t_index = i;
			min_distance = test.distance;
		}
	}

	return min_hit_test;
}

// ------------------------
// > TEST RAY AGAINST MESH
// ------------------------
// This does take a matModel
RaycastTest CL_TestAgainstRay(Ray ray, Mesh* mesh, glm::mat4 mat_model, RayCastType test_type)
{
	int triangles = mesh->indices.size() / 3;
	float min_distance = MaxFloat;
	RaycastTest min_hit_test{false, -1};
	for (int i = 0; i < triangles; i++)
	{
		Triangle t = get_triangle_for_indexed_mesh(mesh, mat_model, i);
		bool test_both_sides = test_type == RayCast_TestBothSidesOfTriangle;
		auto test = CL_TestAgainstRay(ray, t, test_both_sides);
		if (test.hit && test.distance < min_distance)
		{
			min_hit_test = test;
			min_hit_test.t_index = i;
			min_distance = test.distance;
		}
	}

	return min_hit_test;
}

// ----------------------------
// > TEST RAY AGAINST TRIANGLE
// ----------------------------
RaycastTest CL_TestAgainstRay(Ray ray, Triangle triangle, bool test_both_sides)
{
	auto& A = triangle.a;
	auto& B = triangle.b;
	auto& C = triangle.c;
	vec3 E1 = B - A;
	vec3 E2 = C - A;
	vec3 AO = ray.origin - A;

	// check hit with one side of triangle
	vec3 N = cross(E1, E2);
	float det = -dot(ray.direction, N);
	float invdet = 1.0 / det;
	vec3 DAO = cross(AO, ray.direction);
	float u = dot(E2, DAO) * invdet;
	float v = -dot(E1, DAO) * invdet;
	float t = dot(AO, N) * invdet;
	bool test = (det >= 1e-6 && t >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);

	if (!test && test_both_sides)
	{
		// check other side
		N = cross(E2, E1);
		det = -dot(ray.direction, N);
		invdet = 1.0 / det;
		DAO = cross(ray.direction, AO);
		u = dot(E2, DAO) * invdet;
		v = -dot(E1, DAO) * invdet;
		t = dot(AO, N) * invdet;
		test = (det >= 1e-6 && t >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);
	}

	if (test)
	{
		RaycastTest result;
		result.hit = true;
		result.distance = t;
		result.t = triangle;
		result.ray = ray;
		return result;
	}

	return RaycastTest{false, -1};
}

// ---------------
// > CAST PICKRAY
// ---------------
Ray CastPickray(Camera* camera, double screen_x, double screen_y)
{
	float screen_x_normalized = (
		(screen_x - GlobalDisplayConfig::viewport_width / 2)
		/ (GlobalDisplayConfig::viewport_width / 2)
	);
	float screen_y_normalized = (
		-1 * (screen_y - GlobalDisplayConfig::viewport_height / 2)
		/ (GlobalDisplayConfig::viewport_height / 2)
	);

	auto ray_clip = glm::vec4(screen_x_normalized, screen_y_normalized, -1.0, 1.0);
	glm::mat4 inv_view = inverse(camera->mat_view);
	glm::mat4 inv_proj = inverse(camera->mat_projection);
	vec3 ray_eye_3 = (inv_proj * ray_clip);
	auto ray_eye = glm::vec4(ray_eye_3.x, ray_eye_3.y, -1.0, 0.0);
	auto direction = normalize(inv_view * ray_eye);
	auto origin = camera->position;

	return Ray{origin, direction};
}

vec3 CL_GetPointFromDetection(Ray ray, RaycastTest result)
{
	assert(result.hit);
	return ray.origin + ray.direction * result.distance;
}