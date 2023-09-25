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

struct GjkIteration
{
	RSimplex Simplex;
	vec3 Direction;
	bool Finished = false;
};

struct GjkResult
{
	RSimplex Simplex;
	bool Collision = false;
};

struct GjkPoint
{
	vec3 Point;
	bool Empty = false;
};

GjkPoint ClFindFurthestVertex(RCollisionMesh* CollisionMesh, vec3 Direction);
GjkPoint ClGetSupportPoint(RCollisionMesh* CollisionMeshA, RCollisionMesh* CollisionMeshB, vec3 Direction);
void ClUpdateLineSimplex(GjkIteration* Gjk);
void ClUpdateTriangleSimplex(GjkIteration* Gjk);
void ClUpdateTetrahedronSimplex(GjkIteration* Gjk);
void ClUpdateSimplexAndDirection(GjkIteration* Gjk);

void ClDebugRenderSimplex(RSimplex Simplex);
GjkResult ClRunGjk(RCollisionMesh* ColliderA, RCollisionMesh* ColliderB);


inline bool ClSameGeneralDirection(vec3 A, vec3 B)
{
	return glm::dot(A, B) > 0;
}
