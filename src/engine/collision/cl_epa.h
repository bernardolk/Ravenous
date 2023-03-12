// ---------------------------------------------
//       EPA - Expanded Polytope Algorithm
// ---------------------------------------------
// Uses the output of GJK to compute a penetration vector, useful for resolving collisions
#pragma once

struct CollisionMesh;
struct Simplex;

extern const int ClMaxEpaIterations;

struct EPA_Result
{
	bool collision = false;
	float penetration;
	vec3 direction;
};

std::pair<std::vector<vec4>, size_t> CL_GetEPAFaceNormalsAndClosestFace(
	const std::vector<vec3>& polytope,
	const std::vector<size_t>& faces);

void CL_AddIfOuterEdge(
	std::vector<std::pair<size_t, size_t> >& edges,
	const std::vector<size_t>& faces,
	size_t a,
	size_t b);

EPA_Result CL_RunEPA(Simplex simplex, CollisionMesh* collider_a, CollisionMesh* collider_b);


inline bool CL_SupportIsInPolytope(std::vector<vec3> polytope, vec3 support_point)
{
	for (int i = 0; i < polytope.size(); i++)
	{
		if (polytope[i] == support_point)
			return true;
	}

	return false;
}
