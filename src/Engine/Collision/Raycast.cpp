#include <engine/collision/raycast.h>
#include "engine/camera/camera.h"
#include <engine/collision/primitives/BoundingBox.h>
#include "engine/geometry/mesh.h"
#include <glm/gtx/normal.hpp>
#include <engine/geometry/triangle.h>
#include <engine/collision/primitives/ray.h>
#include <glm/gtx/quaternion.hpp>
#include <engine/collision/CollisionMesh.h>

#include "Engine/Geometry/Quad.h"
#include "Engine/IO/Input.h"
#include "engine/entities/Entity.h"
#include "engine/io/display.h"
#include "engine/utils/utils.h"

// --------------------------
// > TEST RAY AGAINST AABB
// --------------------------
bool TestRayAgainstBoundingBox(const RRay& Ray, RBoundingBox Box)
{
	vec3 RayInv = Ray.GetInverse();

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
RRaycastTest TestRayAgainstEntity(const RRay& Ray, EEntity* Entity, NRayCastType TestType)
{
	// @TODO: when testing against player, we could:
	//      a) find the closest point between player's column and the ray
	//      b) do a sphere vs ray test 
	//      instead of testing the collider

	// first check collision with bounding box
	if (TestRayAgainstBoundingBox(Ray, Entity->BoundingBox)) {
		Entity->UpdateCollider();
		return TestRayAgainstCollider(Ray, &Entity->Collider, TestType);
	}
	RRaycastTest Return;
	Return.Hit = false;
	return Return;
}

// ---------------------------
// > TEST RAY AGAINT COLLIDER
// ---------------------------
// This doesn't take a MatModel
RRaycastTest TestRayAgainstCollider(const RRay& Ray, RCollisionMesh* Collider, NRayCastType TestType)
{
	int Triangles = Collider->Indices.size() / 3;
	float MinDistance = MaxFloat;
	RRaycastTest MinHitTest{};
	for (int I = 0; I < Triangles; I++)
	{
		RTriangle T = GetTriangleForColliderIndexedMesh(Collider, I);
		bool TestBothSides = TestType == RayCast_TestBothSidesOfTriangle;
		auto Test = TestRayAgainstTriangle(Ray, T, TestBothSides);
		if (Test.Hit && Test.Distance < MinDistance) {
			MinHitTest = Test;
			MinDistance = Test.Distance;
		}
	}

	return MinHitTest;
}

// ------------------------
// > TEST RAY AGAINST MESH
// ------------------------
// This does take a matModel
RRaycastTest TestRayAgainstMesh(const RRay& Ray, RMesh* Mesh, glm::mat4 MatModel, NRayCastType TestType)
{
	int Triangles = Mesh->Indices.size() / 3;
	float MinDistance = MaxFloat;
	RRaycastTest MinHitTest{};
	for (int i = 0; i < Triangles; i++)
	{
		RTriangle T = GetTriangleForIndexedMesh(Mesh, MatModel, i);
		bool TestBothSides = TestType == RayCast_TestBothSidesOfTriangle;
		auto Test = TestRayAgainstTriangle(Ray, T, TestBothSides);
		if (Test.Hit && Test.Distance < MinDistance) {
			MinHitTest = Test;
			MinDistance = Test.Distance;
		}
	}

	return MinHitTest;
}

// ----------------------------
// > TEST RAY AGAINST TRIANGLE
// ----------------------------
RRaycastTest TestRayAgainstTriangle(const RRay& Ray, RTriangle Triangle, bool TestBothSides)
{
	RRaycastTest Result;
	Result.Hit = false;
	Result.Distance = -1;
	
	auto& A = Triangle.A;
	auto& B = Triangle.B;
	auto& C = Triangle.C;
	vec3 E1 = B - A;
	vec3 E2 = C - A;
	vec3 AO = Ray.Origin - A;

	// check hit with one side of triangle
	vec3 N = cross(E1, E2);
	float Det = -dot(Ray.Direction, N);
	float Invdet = 1.0f / Det;
	vec3 DAO = cross(AO, Ray.Direction);
	float U = dot(E2, DAO) * Invdet;
	float V = -dot(E1, DAO) * Invdet;
	float T = dot(AO, N) * Invdet;
	bool Test = (Det >= 1e-6 && T >= 0.0 && U >= 0.0 && V >= 0.0 && (U + V) <= 1.0);

	if (!Test && TestBothSides) {
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

	if (Test) {
		Result.Hit = true;
		Result.Distance = T;
		Result.Triangle = Triangle;
		Result.Ray = Ray;
		return Result;
	}

	return Result;
}

RRaycastTest TestRayAgainstQuad(const RRay& Ray, const RQuad& Quad, bool TestBothSides)
{
	auto T1RayTest = TestRayAgainstTriangle(Ray, Quad.T1, TestBothSides);
	if (T1RayTest.Hit) return T1RayTest;
	
	auto T2RayTest = TestRayAgainstTriangle(Ray, Quad.T2, TestBothSides);
	if (T2RayTest.Hit) return T2RayTest;

	return {};
}

// ---------------
// > CAST PICKRAY
// ---------------
RRay CastPickray(RCamera* Camera)
{
	if (!Camera) {
		Camera = RCameraManager::Get()->GetCurrentCamera();
	}
	
	double ScreenX = Camera->MouseCoordinates.X;
	double ScreenY = Camera->MouseCoordinates.Y;
	float ScreenXNormalized = ((ScreenX - GlobalDisplayState::ViewportWidth / 2) / (GlobalDisplayState::ViewportWidth / 2));
	float ScreenYNormalized = (-1 * (ScreenY - GlobalDisplayState::ViewportHeight / 2) / (GlobalDisplayState::ViewportHeight / 2));
	auto RayClip = vec4(ScreenXNormalized, ScreenYNormalized, -1.0, 1.0);
	mat4 InvView = Inverse(Camera->MatView);
	mat4 InvProj = Inverse(Camera->MatProjection);
	vec3 RayEye3 = (InvProj * RayClip);
	auto RayEye = vec4(RayEye3.x, RayEye3.y, -1.0, 0.0);
	auto Direction = Normalize(InvView * RayEye);
	auto Origin = Camera->Position;

	return RRay{Origin, Direction};
}

RRay CastFirstPersonRay()
{
	auto* Camera = RCameraManager::Get()->GetGameCamera();
	auto RayClip = vec4(0.5f, 0.5f, -1.0, 1.0);
	mat4 InvView = Inverse(Camera->MatView);
	mat4 InvProj = Inverse(Camera->MatProjection);
	vec3 RayEye3 = (InvProj * RayClip);
	auto RayEye = vec4(RayEye3.x, RayEye3.y, -1.0, 0.0);
	return RRay{Camera->Position, Normalize(InvView * RayEye)};
}
