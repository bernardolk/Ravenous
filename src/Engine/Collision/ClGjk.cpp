#include "ClGjk.h"

#include "CollisionMesh.h"
#include "engine/utils/colors.h"
#include "engine/render/ImRender.h"

vec3 DebugColors[] = {
COLOR_RED_1,
COLOR_GREEN_1,
COLOR_BLUE_1,
COLOR_PURPLE_1,
};

/* -----------------------
  GJK Support Functions
----------------------- */
GjkPoint ClFindFurthestVertex(RCollisionMesh* CollisionMesh, vec3 Direction)
{
	// Linearly scan the CollisionMesh doing dot products with the vertices and storing the one with max value, then return it
	// Note: sometimes, the dot product between two points equals the same, but there is always a right and a wrong pair 
	// of Support points that are ideal to go together. We are not storing these 'dispute points' and testing between them for
	// the sake of simplicity. If anything, it should take a bit more iteration to find a solution but it should work either
	// way.

	float MaxInnerP = MinFloat;
	vec3 FurthestVertex{0.f};

	for (int i = 0; i < CollisionMesh->Vertices.size(); i++)
	{
		vec3 VertexPos = CollisionMesh->Vertices[i];
		float InnerP = dot(VertexPos, Direction);
		if (InnerP > MaxInnerP)
		{
			MaxInnerP = InnerP;
			FurthestVertex = VertexPos;
		}
	}

	return GjkPoint{FurthestVertex, MaxInnerP == MinFloat};
}


GjkPoint ClGetSupportPoint(RCollisionMesh* CollisionMeshA, RCollisionMesh* CollisionMeshB, vec3 Direction)
{
	// Gets a Support point in the minkowski difference of both meshes, in the Direction supplied.

	// PRIOR METHOD
	GjkPoint GjkPointA = ClFindFurthestVertex(CollisionMeshA, Direction);
	GjkPoint GjkPointB = ClFindFurthestVertex(CollisionMeshB, -Direction);

	if (GjkPointA.Empty || GjkPointB.Empty)
		return GjkPointA;

	vec3 GjkSupportPoint = GjkPointA.Point - GjkPointB.Point;

	// NEW METHOD
	/* GjkPoint GjkPointA = Clfind_furthest_vertex(collision_mesh_A, Direction);
	 GjkPoint GjkPointB = Clfind_furthest_vertex(collision_mesh_B, -Direction);
  
	 if (GjkPointA.Empty || GjkPointB.Empty)
	    return GjkPointA;
  
	 vec3 gjk_Support_point = GjkPointA.Point - GjkPointB.Point;*/

	return GjkPoint{GjkSupportPoint, false};
}

/* -----------------------
  Update simplex calls
----------------------- */
void ClUpdateLineSimplex(GjkIteration* Gjk)
{
	auto& A = Gjk->Simplex[0];
	auto& B = Gjk->Simplex[1];

	vec3 Ab = B - A;
	vec3 Ao = - A;

	if (ClSameGeneralDirection(Ab, Ao))
	{
		Gjk->Direction = Cross(Ab, Ao, Ab);
	}
	else
	{
		Gjk->Simplex = {A};
		Gjk->Direction = Ao;
	}
}


void ClUpdateTriangleSimplex(GjkIteration* Gjk)
{
	auto& A = Gjk->Simplex[0];
	auto& B = Gjk->Simplex[1];
	auto& C = Gjk->Simplex[2];

	vec3 Ab = B - A;
	vec3 Ac = C - A;
	vec3 Ao = -A;

	vec3 Abc = glm::cross(Ab, Ac);

	if (ClSameGeneralDirection(glm::cross(Abc, Ac), Ao))
	{
		if (ClSameGeneralDirection(Ac, Ao))
		{
			// search in purple region
			Gjk->Simplex = {A, C};
			Gjk->Direction = Cross(Ac, Ao, Ac);
		}
		else
		{
			// search in navy/blue grey region
			Gjk->Simplex = {A, B};
			Gjk->Direction = -Ac + (-Ab);
		}
	}
	else
	{
		if (ClSameGeneralDirection(glm::cross(Ab, Abc), Ao))
		{
			// search in cyan region
			Gjk->Simplex = {A, B};
			Gjk->Direction = glm::cross(Ab, Abc);
		}
		else
		{
			if (ClSameGeneralDirection(Abc, Ao))
			{
				// search up
				Gjk->Direction = Abc;
			}
			else
			{
				// search down
				Gjk->Simplex = {A, C, B};
				Gjk->Direction = -Abc;
			}
		}
	}
}


void ClUpdateTetrahedronSimplex(GjkIteration* Gjk)
{
	auto& A = Gjk->Simplex[0];
	auto& B = Gjk->Simplex[1];
	auto& C = Gjk->Simplex[2];
	auto& D = Gjk->Simplex[3];

	vec3 Ab = B - A;
	vec3 Ac = C - A;
	vec3 Ad = D - A;
	vec3 Ao = - A;

	vec3 Abc = glm::cross(Ab, Ac);
	vec3 Acd = glm::cross(Ac, Ad);
	vec3 Adb = glm::cross(Ad, Ab);

	// check if mikowski's diff origin is pointing towards tetrahedron normal faces
	// (it shouldn't, since the origin should be contained in the shape and point down not up like the inclined faces's normals)

	if (ClSameGeneralDirection(Abc, Ao))
	{
		Gjk->Simplex = {A, B, C};
		Gjk->Direction = Abc;
		return ClUpdateTriangleSimplex(Gjk);
	}

	if (ClSameGeneralDirection(Acd, Ao))
	{
		Gjk->Simplex = {A, C, D};
		Gjk->Direction = Acd;
		return ClUpdateTriangleSimplex(Gjk);
	}

	if (ClSameGeneralDirection(Adb, Ao))
	{
		Gjk->Simplex = {A, D, B};
		Gjk->Direction = Adb;
		return ClUpdateTriangleSimplex(Gjk);
	}

	// was not the case, found collision
	Gjk->Finished = true;
}


void ClUpdateSimplexAndDirection(GjkIteration* Gjk)
{
	switch (Gjk->Simplex.size())
	{
		case 2:
			return ClUpdateLineSimplex(Gjk);
		case 3:
			return ClUpdateTriangleSimplex(Gjk);
		case 4:
			return ClUpdateTetrahedronSimplex(Gjk);
	}

	// something went wrong
	assert(false);
}


/* ------------------
   Run GJK
------------------ */

void ClDebugRenderSimplex(RSimplex Simplex)
{
	for (int i = 0; i < Simplex.size(); i++)
		RImDraw::AddPoint(IM_ITERHASH(i), Simplex[i], 2.0, true, DebugColors[i]);
}


GjkResult ClRunGjk(RCollisionMesh* ColliderA, RCollisionMesh* ColliderB)
{
	GjkPoint Support = ClGetSupportPoint(ColliderA, ColliderB, UnitX);

	if (Support.Empty)
		return {};

	GjkIteration Gjk;
	Gjk.Simplex.PushFront(Support.Point);
	Gjk.Direction = -Support.Point;

	// ImDraw::add_point(IMHASH, Support.Point, 2.0, true, Debug_Colors[0]);

	int ItCount = 0;
	while (true)
	{
		Support = ClGetSupportPoint(ColliderA, ColliderB, Gjk.Direction);
		if (Support.Empty || !ClSameGeneralDirection(Support.Point, Gjk.Direction))
		{
			// _Cldebug_render_simplex(gjk.Simplex);
			return {}; // no collision
		}

		Gjk.Simplex.PushFront(Support.Point);

		// ImDraw::add_point(IM_ITERHASH(it_count), Support.Point, 2.0, true, Debug_Colors[it_count + 1]);

		ClUpdateSimplexAndDirection(&Gjk);

		ItCount++;
		if (Gjk.Finished)
		{
			//_Cldebug_render_simplex(gjk.Simplex);
			return {.Simplex = Gjk.Simplex, .Collision = true};
		}
	}
}
