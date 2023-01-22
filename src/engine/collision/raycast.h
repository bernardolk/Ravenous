#pragma once

struct Triangle;
struct Ray;
struct Mesh;
struct BoundingBox;
struct Entity;

struct RaycastTest
{
	bool hit = false;
	float distance = 0;
	Entity* entity = nullptr;
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

Ray cast_pickray(Camera* camera, double screen_x, double screen_y);
RaycastTest test_ray_against_entity(Ray ray, Entity* entity, RayCastType test_type, float max_distance);
RaycastTest test_ray_against_entity(Ray ray, Entity* entity);
RaycastTest test_ray_against_mesh(Ray ray, Mesh* mesh, glm::mat4 mat_model, RayCastType test_type);
RaycastTest test_ray_against_triangle(Ray ray, Triangle triangle, bool test_both_sides = true);
RaycastTest test_ray_against_collider(Ray ray, CollisionMesh* collider, RayCastType test_type);
vec3 point_from_detection(Ray ray, RaycastTest result);
vec3 get_triangle_normal(Triangle t);
bool test_ray_against_aabb(Ray ray, BoundingBox box);
