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

std::pair<std::vector<vec4>, uint> CL_GetEPAFaceNormalsAndClosestFace(
	const std::vector<vec3>& polytope,
	const std::vector<uint>& faces);

void CL_AddIfOuterEdge(
	std::vector<std::pair<uint, uint> >& edges,
	const std::vector<uint>& faces,
	uint a,
	uint b);

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
