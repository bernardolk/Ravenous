#include <engine/collision/raycast.h>
#include "engine/camera/camera.h"
#include <engine/collision/primitives/BoundingBox.h>
#include "engine/geometry/mesh.h"
#include <glm/gtx/normal.hpp>
#include <engine/geometry/triangle.h>
#include <engine/collision/primitives/ray.h>
#include <glm/gtx/quaternion.hpp>
#include <engine/collision/CollisionMesh.h>
#include "engine/entities/Entity.h"
#include "engine/io/display.h"
#include "engine/utils/utils.h"

// --------------------------
// > TEST RAY AGAINST AABB
// --------------------------
bool ClTestAgainstRay(RRay Ray, RBoundingBox Box)
{
	vec3 RayInv = Ray.GetInv();

	float Tx1 = (Box.MinX - Ray.Origin.x) * RayInv.x;
	float Tx2 = (Box.MaxX - Ray.Origin.x) * RayInv.x;

	float Tmin = Min(Tx1, Tx2);
	float Tmax = Max(Tx1, Tx2);

	float Ty1 = (Box.MinY - Ray.Origin.y) * RayInv.y;
	float Ty2 = (Box.MaxY - Ray.Origin.y) * RayInv.y;

	Tmin = Max(Tmin, Min(Ty1, Ty2));
	Tmax = Min(Tmax, Max(Ty1, Ty2));

	float Tz1 = (Box.MinZ - Ray.Origin.z) * RayInv.z;
	float Tz2 = (Box.MaxZ - Ray.Origin.z) * RayInv.z;

	Tmin = Max(Tmin, Min(Tz1, Tz2));
	Tmax = Min(Tmax, Max(Tz1, Tz2));

	return Tmax >= Tmin;
}

// --------------------------
// > TEST RAY AGAINST ENTITY
// --------------------------
RRaycastTest ClTestAgainstRay(
	RRay Ray, EEntity* Entity,
	NRayCastType TestType = RayCast_TestOnlyFromOutsideIn,
	float MaxDistance = MaxFloat)
{
	// @TODO: when testing against player, we could:
	//      a) find the closest point between player's column and the ray
	//      b) do a sphere vs ray test 
	//      instead of testing the collider


	// first check collision with bounding box
	if (ClTestAgainstRay(Ray, Entity->BoundingBox))
	{
		//@TODO: We are not updating player's collider everytime now, so we must do it now on a raycast call
		Entity->UpdateCollider();
		return ClTestAgainstRay(Ray, &Entity->Collider, TestType);
	}

	return RRaycastTest{false};
}


RRaycastTest ClTestAgainstRay(RRay Ray, EEntity* Entity)
{
	return ClTestAgainstRay(Ray, Entity, RayCast_TestOnlyFromOutsideIn, MaxFloat);
}

// ---------------------------
// > TEST RAY AGAINT COLLIDER
// ---------------------------
// This doesn't take a matModel
RRaycastTest ClTestAgainstRay(RRay Ray, RCollisionMesh* Collider, NRayCastType TestType)
{

	int Triangles = Collider->Indices.size() / 3;
	float MinDistance = MaxFloat;
	RRaycastTest MinHitTest{false, -1};
	for (int I = 0; I < Triangles; I++)
	{
		RTriangle T = get_triangle_for_collider_indexed_mesh(Collider, I);
		bool TestBothSides = TestType == RayCast_TestBothSidesOfTriangle;
		auto Test = ClTestAgainstRay(Ray, T, TestBothSides);
		if (Test.Hit && Test.Distance < MinDistance)
		{
			MinHitTest = Test;
			MinHitTest.TriangleIndex = I;
			MinDistance = Test.Distance;
		}
	}

	return MinHitTest;
}

// ------------------------
// > TEST RAY AGAINST MESH
// ------------------------
// This does take a matModel
RRaycastTest ClTestAgainstRay(RRay Ray, RMesh* Mesh, glm::mat4 MatModel, NRayCastType TestType)
{
	int Triangles = Mesh->Indices.size() / 3;
	float MinDistance = MaxFloat;
	RRaycastTest MinHitTest{false, -1};
	for (int i = 0; i < Triangles; i++)
	{
		RTriangle T = get_triangle_for_indexed_mesh(Mesh, MatModel, i);
		bool TestBothSides = TestType == RayCast_TestBothSidesOfTriangle;
		auto Test = ClTestAgainstRay(Ray, T, TestBothSides);
		if (Test.Hit && Test.Distance < MinDistance)
		{
			MinHitTest = Test;
			MinHitTest.TriangleIndex = i;
			MinDistance = Test.Distance;
		}
	}

	return MinHitTest;
}

// ----------------------------
// > TEST RAY AGAINST TRIANGLE
// ----------------------------
RRaycastTest ClTestAgainstRay(RRay Ray, RTriangle Triangle, bool TestBothSides)
{
	auto& A = Triangle.A;
	auto& B = Triangle.B;
	auto& C = Triangle.C;
	vec3 E1 = B - A;
	vec3 E2 = C - A;
	vec3 AO = Ray.Origin - A;

	// check hit with one side of triangle
	vec3 N = cross(E1, E2);
	float Det = -dot(Ray.Direction, N);
	float Invdet = 1.0 / Det;
	vec3 DAO = cross(AO, Ray.Direction);
	float U = dot(E2, DAO) * Invdet;
	float V = -dot(E1, DAO) * Invdet;
	float T = dot(AO, N) * Invdet;
	bool Test = (Det >= 1e-6 && T >= 0.0 && U >= 0.0 && V >= 0.0 && (U + V) <= 1.0);

	if (!Test && TestBothSides)
	{
		// check other side
		N = cross(E2, E1);
		Det = -dot(Ray.Direction, N);
		Invdet = 1.0 / Det;
		DAO = cross(Ray.Direction, AO);
		U = dot(E2, DAO) * Invdet;
		V = -dot(E1, DAO) * Invdet;
		T = dot(AO, N) * Invdet;
		Test = (Det >= 1e-6 && T >= 0.0 && U >= 0.0 && V >= 0.0 && (U + V) <= 1.0);
	}

	if (Test)
	{
		RRaycastTest Result;
		Result.Hit = true;
		Result.Distance = T;
		Result.Triangle = Triangle;
		Result.Ray = Ray;
		return Result;
	}

	return RRaycastTest{false, -1};
}

// ---------------
// > CAST PICKRAY
// ---------------
RRay CastPickray(RCamera* Camera, double ScreenX, double ScreenY)
{
	float ScreenXNormalized = (
		(ScreenX - GlobalDisplayState::ViewportWidth / 2)
		/ (GlobalDisplayState::ViewportWidth / 2)
	);
	float ScreenYNormalized = (
		-1 * (ScreenY - GlobalDisplayState::ViewportHeight / 2)
		/ (GlobalDisplayState::ViewportHeight / 2)
	);

	auto RayClip = glm::vec4(ScreenXNormalized, ScreenYNormalized, -1.0, 1.0);
	glm::mat4 InvView = glm::inverse(Camera->MatView);
	glm::mat4 InvProj = glm::inverse(Camera->MatProjection);
	vec3 RayEye3 = (InvProj * RayClip);
	auto RayEye = glm::vec4(RayEye3.x, RayEye3.y, -1.0, 0.0);
	auto Direction = glm::normalize(InvView * RayEye);
	auto Origin = Camera->Position;

	return RRay{Origin, Direction};
}

vec3 ClGetPointFromDetection(RRay Ray, RRaycastTest Result)
{
	assert(Result.Hit);
	return Ray.Origin + Ray.Direction * Result.Distance;
}
