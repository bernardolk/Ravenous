// ---------------------------------------------
//       EPA - Expanded Polytope Algorithm
// ---------------------------------------------
// Uses the output of GJK to compute a penetration vector, useful for resolving collisions
#pragma once

struct RCollisionMesh;
struct RSimplex;

extern const int ClMaxEpaIterations;

struct EPA_Result
{
	bool collision = false;
	float penetration;
	vec3 direction;
};

std::pair<std::vector<vec4>, u32> CL_GetEPAFaceNormalsAndClosestFace(
	const std::vector<vec3>& polytope,
	const std::vector<u32>& faces);

void CL_AddIfOuterEdge(
	std::vector<std::pair<u32, u32> >& edges,
	const std::vector<u32>& faces,
	u32 a,
	u32 b);

EPA_Result CL_RunEPA(RSimplex simplex, RCollisionMesh* collider_a, RCollisionMesh* collider_b);


inline bool CL_SupportIsInPolytope(std::vector<vec3> polytope, vec3 support_point)
{
	for (int i = 0; i < polytope.size(); i++)
	{
		if (polytope[i] == support_point)
			return true;
	}

	return false;
}
