#pragma once

// -----------------------------------------------
//    GJK - Gilbert–Johnson–Keerthi Algorithm
// -----------------------------------------------
// Utilizes the fact that the Minkowski difference's shape of two meshes will contain
// the origin if they are intersecting, and it won't in case not.
// 
// props to blog.winter.dev for the excellent explanation of the algorithm and code examples.
//
// Here, we define data structures to be used by the algorithm and auxiliary functions for the
// main looping function. Each iteration we find a point in the minkowski difference that is
// a candidate for composing a simplex (simplest geom. form in N-dimensions [line, triangle,
// tetrahedron, ...]) that will enclose the origin and check if we should change direction or
// discard one or another point inside the simplex by computing where the origin is in relation
// to each face/region delimited by the current simplex. The direction mentioned is the direction
// in which we will try picking the furthest point in the minkowski's difference shape.

#include "engine/core/core.h"
#include "simplex.h"

struct GJK_Iteration
{
	Simplex simplex;
	vec3 direction;
	bool finished = false;
};

struct GJK_Result
{
	Simplex simplex;
	bool collision = false;
};

struct GJK_Point
{
	vec3 point;
	bool empty;
};

GJK_Point CL_find_furthest_vertex(CollisionMesh* collision_mesh, vec3 direction);
GJK_Point CL_get_support_point(CollisionMesh* collision_mesh_a, CollisionMesh* collision_mesh_b, vec3 direction);
void CL_update_line_simplex(GJK_Iteration* gjk);
void CL_update_triangle_simplex(GJK_Iteration* gjk);
void CL_update_tetrahedron_simplex(GJK_Iteration* gjk);
void CL_update_simplex_and_direction(GJK_Iteration* gjk);

void _CL_debug_render_simplex(Simplex simplex);
GJK_Result CL_run_GJK(CollisionMesh* collider_a, CollisionMesh* collider_b);


inline bool CL_same_general_direction(vec3 a, vec3 b)
{
	return dot(a, b) > 0;
}
