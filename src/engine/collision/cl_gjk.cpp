#include "cl_gjk.h"

#include "collision_mesh.h"
#include "engine/utils/colors.h"
#include "engine/render/im_render.h"

vec3 DebugColors[] = {
COLOR_RED_1,
COLOR_GREEN_1,
COLOR_BLUE_1,
COLOR_PURPLE_1,
};

/* -----------------------
  GJK Support Functions
----------------------- */
GJK_Point CL_FindFurthestVertex(CollisionMesh* collision_mesh, vec3 direction)
{
	// Linearly scan the CollisionMesh doing dot products with the vertices and storing the one with max value, then return it
	// Note: sometimes, the dot product between two points equals the same, but there is always a right and a wrong pair 
	// of support points that are ideal to go together. We are not storing these 'dispute points' and testing between them for
	// the sake of simplicity. If anything, it should take a bit more iteration to find a solution but it should work either
	// way.

	float max_inner_p = MinFloat;
	vec3 furthest_vertex{};

	for (int i = 0; i < collision_mesh->vertices.size(); i++)
	{
		vec3 vertex_pos = collision_mesh->vertices[i];
		float inner_p = dot(vertex_pos, direction);
		if (inner_p > max_inner_p)
		{
			max_inner_p = inner_p;
			furthest_vertex = vertex_pos;
		}
	}

	return GJK_Point{furthest_vertex, max_inner_p == MinFloat};
}


GJK_Point CL_GetSupportPoint(CollisionMesh* collision_mesh_a, CollisionMesh* collision_mesh_b, vec3 direction)
{
	// Gets a support point in the minkowski difference of both meshes, in the direction supplied.

	// PRIOR METHOD
	GJK_Point gjk_point_a = CL_FindFurthestVertex(collision_mesh_a, direction);
	GJK_Point gjk_point_b = CL_FindFurthestVertex(collision_mesh_b, -direction);

	if (gjk_point_a.empty || gjk_point_b.empty)
		return gjk_point_a;

	vec3 gjk_support_point = gjk_point_a.point - gjk_point_b.point;

	// NEW METHOD
	/* GJK_Point gjk_point_a = CL_find_furthest_vertex(collision_mesh_A, direction);
	 GJK_Point gjk_point_b = CL_find_furthest_vertex(collision_mesh_B, -direction);
  
	 if (gjk_point_a.empty || gjk_point_b.empty)
	    return gjk_point_a;
  
	 vec3 gjk_support_point = gjk_point_a.point - gjk_point_b.point;*/

	return GJK_Point{gjk_support_point, false};
}

/* -----------------------
  Update simplex calls
----------------------- */
void CL_UpdateLineSimplex(GJK_Iteration* gjk)
{
	auto& a = gjk->simplex[0];
	auto& b = gjk->simplex[1];

	vec3 ab = b - a;
	vec3 ao = - a;

	if (CL_SameGeneralDirection(ab, ao))
	{
		gjk->direction = Cross(ab, ao, ab);
	}
	else
	{
		gjk->simplex = {a};
		gjk->direction = ao;
	}
}


void CL_UpdateTriangleSimplex(GJK_Iteration* gjk)
{
	auto& a = gjk->simplex[0];
	auto& b = gjk->simplex[1];
	auto& c = gjk->simplex[2];

	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 ao = - a;

	vec3 abc = glm::cross(ab, ac);

	if (CL_SameGeneralDirection(glm::cross(abc, ac), ao))
	{
		if (CL_SameGeneralDirection(ac, ao))
		{
			// search in purple region
			gjk->simplex = {a, c};
			gjk->direction = Cross(ac, ao, ac);
		}
		else
		{
			// search in navy/blue grey region
			gjk->simplex = {a, b};
			gjk->direction = -ac + (-ab);
		}
	}
	else
	{
		if (CL_SameGeneralDirection(glm::cross(ab, abc), ao))
		{
			// search in cyan region
			gjk->simplex = {a, b};
			gjk->direction = glm::cross(ab, abc);
		}
		else
		{
			if (CL_SameGeneralDirection(abc, ao))
			{
				// search up
				gjk->direction = abc;
			}
			else
			{
				// search down
				gjk->simplex = {a, c, b};
				gjk->direction = -abc;
			}
		}
	}
}


void CL_UpdateTetrahedronSimplex(GJK_Iteration* gjk)
{
	auto& a = gjk->simplex[0];
	auto& b = gjk->simplex[1];
	auto& c = gjk->simplex[2];
	auto& d = gjk->simplex[3];

	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 ad = d - a;
	vec3 ao = - a;

	vec3 abc = glm::cross(ab, ac);
	vec3 acd = glm::cross(ac, ad);
	vec3 adb = glm::cross(ad, ab);

	// check if mikowski's diff origin is pointing towards tetrahedron normal faces
	// (it shouldn't, since the origin should be contained in the shape and point down not up like the inclined faces's normals)

	if (CL_SameGeneralDirection(abc, ao))
	{
		gjk->simplex = {a, b, c};
		gjk->direction = abc;
		return CL_UpdateTriangleSimplex(gjk);
	}

	if (CL_SameGeneralDirection(acd, ao))
	{
		gjk->simplex = {a, c, d};
		gjk->direction = acd;
		return CL_UpdateTriangleSimplex(gjk);
	}

	if (CL_SameGeneralDirection(adb, ao))
	{
		gjk->simplex = {a, d, b};
		gjk->direction = adb;
		return CL_UpdateTriangleSimplex(gjk);
	}

	// was not the case, found collision
	gjk->finished = true;
}


void CL_UpdateSimplexAndDirection(GJK_Iteration* gjk)
{
	switch (gjk->simplex.size())
	{
		case 2:
			return CL_UpdateLineSimplex(gjk);
		case 3:
			return CL_UpdateTriangleSimplex(gjk);
		case 4:
			return CL_UpdateTetrahedronSimplex(gjk);
	}

	// something went wrong
	assert(false);
}


/* ------------------
   Run GJK
------------------ */

void CL_DebugRenderSimplex(Simplex simplex)
{
	for (int i = 0; i < simplex.size(); i++)
		ImDraw::AddPoint(IM_ITERHASH(i), simplex[i], 2.0, true, DebugColors[i]);
}


GJK_Result CL_RunGjk(CollisionMesh* collider_a, CollisionMesh* collider_b)
{
	GJK_Point support = CL_GetSupportPoint(collider_a, collider_b, UnitX);

	if (support.empty)
		return {};

	GJK_Iteration gjk;
	gjk.simplex.PushFront(support.point);
	gjk.direction = -support.point;

	// ImDraw::add_point(IMHASH, support.point, 2.0, true, Debug_Colors[0]);

	int it_count = 0;
	while (true)
	{
		support = CL_GetSupportPoint(collider_a, collider_b, gjk.direction);
		if (support.empty || !CL_SameGeneralDirection(support.point, gjk.direction))
		{
			// _CL_debug_render_simplex(gjk.simplex);
			return {}; // no collision
		}

		gjk.simplex.PushFront(support.point);

		// ImDraw::add_point(IM_ITERHASH(it_count), support.point, 2.0, true, Debug_Colors[it_count + 1]);

		CL_UpdateSimplexAndDirection(&gjk);

		it_count++;
		if (gjk.finished)
		{
			//_CL_debug_render_simplex(gjk.simplex);
			return {.simplex = gjk.simplex, .collision = true};
		}
	}
}
