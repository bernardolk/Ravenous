// ---------------------------------------------
//       EPA - Expanded Polytope Algorithm
// ---------------------------------------------
// Uses the output of GJK to compute a penetration vector, useful for resolving collisions
#pragma once

struct RCollisionMesh;
struct RSimplex;

extern const int ClMaxEpaIterations;

struct EpaResult
{
	bool Collision = false;
	float Penetration;
	vec3 Direction;
};

std::pair<vector<vec4>, uint> ClGetEPAFaceNormalsAndClosestFace(const vector<vec3>& Polytope, const vector<uint>& Faces);

void ClAddIfOuterEdge(vector<std::pair<uint, uint> >& Edges,const vector<uint>& Faces, uint A, uint B);

EpaResult ClRunEpa(RSimplex Simplex, RCollisionMesh* ColliderA, RCollisionMesh* ColliderB);


inline bool ClSupportIsInPolytope(vector<vec3> Polytope, vec3 SupportPoint)
{
	for (int i = 0; i < Polytope.size(); i++)
	{
		if (Polytope[i] == SupportPoint)
			return true;
	}

	return false;
}
