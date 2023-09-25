#include <engine/core/types.h>
#include <engine/collision/simplex.h>
#include <engine/collision/ClGjk.h>
#include <engine/collision/ClEpa.h>

extern const int ClMaxEpaIterations = 100;

std::pair<vector<vec4>, uint> ClGetEPAFaceNormalsAndClosestFace(const vector<vec3>& Polytope, const vector<uint>& Faces)
{
	vector<vec4> Normals;
	uint ClosestFaceIndex = 0;
	float MinDistanceToFace = MaxFloat;

	for (uint i = 0; i < Faces.size(); i += 3)
	{
		vec3 A = Polytope[Faces[i]];
		vec3 B = Polytope[Faces[i + 1]];
		vec3 C = Polytope[Faces[i + 2]];

		vec3 Normal = normalize(glm::cross(B - A, C - A));
		float Distance = dot(Normal, C);

		if (Distance < 0)
		{
			Normal *= -1;
			Distance *= -1;
		}

		Normals.emplace_back(Normal, Distance);

		if (Distance < MinDistanceToFace)
		{
			ClosestFaceIndex = i / 3;
			MinDistanceToFace = Distance;
		}
	}

	return {Normals, ClosestFaceIndex};
}


void ClAddIfOuterEdge(vector<std::pair<uint, uint> >& Edges, const vector<uint>& Faces, uint A, uint B)
{
	// if edge is already in list (but in reverse winding order)
	// then we must exclude it from the list as it is not an outer edge.
	// if we don't find it, just add.

	auto Reverse = std::find(              //      0--<--3
		Edges.begin(),                     //     / \ B /   A: 2-0
		Edges.end(),                       //    / A \ /    B: 0-2
		std::make_pair(Faces[B], Faces[A]) //   1-->--2
	);

	if (Reverse != Edges.end())
		Edges.erase(Reverse);
	else
		Edges.emplace_back(Faces[A], Faces[B]);
}

EpaResult ClRunEpa(RSimplex Simplex, RCollisionMesh* ColliderA, RCollisionMesh* ColliderB)
{

	vector<vec3> Polytope;
	Polytope.insert(Polytope.begin(), std::begin(Simplex.Points), std::end(Simplex.Points));

	vector<uint> Faces = {
		0, 1, 2,
		0, 3, 1,
		0, 2, 3,
		1, 3, 2
	};

	auto [face_normals, closest_face_index] = ClGetEPAFaceNormalsAndClosestFace(Polytope, Faces);

	vec3 PenetrationNormal;
	float MinDistanceToFace = MaxFloat;

	int EPAIterations = 0;

	while (MinDistanceToFace == MaxFloat && EPAIterations < ClMaxEpaIterations)
	{
		EPAIterations++;
		PenetrationNormal = vec3(face_normals[closest_face_index]);
		MinDistanceToFace = face_normals[closest_face_index].w;

		GjkPoint Support = ClGetSupportPoint(ColliderA, ColliderB, PenetrationNormal);
		if (Support.Empty || ClSupportIsInPolytope(Polytope, Support.Point))
			break;

		float SDistance = dot(PenetrationNormal, Support.Point);

		// if we have a vertex in support direction and its farther enough to check
		if (abs(SDistance - MinDistanceToFace) > 0.001f)
		{
			MinDistanceToFace = MaxFloat;

			// removes all faces pointing towards the support direction and lists the outer_edges
			vector<std::pair<uint, uint> > OuterEdges;
			for (uint i = 0; i < face_normals.size(); i++)
			{
				if (ClSameGeneralDirection(face_normals[i], Support.Point))
				{
					uint f = i * 3;

					ClAddIfOuterEdge(OuterEdges, Faces, f, f + 1);
					ClAddIfOuterEdge(OuterEdges, Faces, f + 1, f + 2);
					ClAddIfOuterEdge(OuterEdges, Faces, f + 2, f);

					Faces[f + 2] = Faces.back();
					Faces.pop_back();
					Faces[f + 1] = Faces.back();
					Faces.pop_back();
					Faces[f] = Faces.back();
					Faces.pop_back();

					face_normals[i] = face_normals.back();
					face_normals.pop_back();

					i--;
				}
			}

			// construct new faces from the outer_edges listed before
			vector<uint> NewFaces;
			for (auto [edgeIndex1, edgeIndex2] : OuterEdges)
			{
				NewFaces.push_back(edgeIndex1);
				NewFaces.push_back(edgeIndex2);
				NewFaces.push_back(Polytope.size());
			}

			Polytope.push_back(Support.Point);

			// finds normals and closest face from the new faces
			auto [new_normals, new_closest_face_index] = ClGetEPAFaceNormalsAndClosestFace(Polytope, NewFaces);

			float OldMinDistanceToFace = MaxFloat;
			for (uint i = 0; i < face_normals.size(); i++)
			{
				float dist = face_normals[i].w;
				if (dist < OldMinDistanceToFace)
				{
					OldMinDistanceToFace = dist;
					closest_face_index = i;
				}
			}

			if (new_normals[new_closest_face_index].w < OldMinDistanceToFace)
				closest_face_index = new_closest_face_index + face_normals.size();

			Faces.insert(Faces.end(), NewFaces.begin(), NewFaces.end());
			face_normals.insert(face_normals.end(), new_normals.begin(), new_normals.end());
		}
	}

	// // DEBUG
	// if(false)
	// {
	//    // RENDER POLYTOPE
	//    for (int i = 0; i < polytope.size(); i++)
	//    {
	//       ImDraw::add_point(
	//          IMCUSTOMHASH("poly-" + std::to_string(i)), 
	//          polytope[i], 2.0, true, vec3(0.4, 0.2, 0.4), 1
	//       );

	//       for (int j = 0; j < polytope.size(); j++)
	//          if (i != j && glm::length(polytope[i] - polytope[j]) <= 0.001)
	//             RVN::print_dynamic("POINTS " + std::to_string(i) + " AND " + std::to_string(j) + " ARE EQUAL", 2000);
	//    }

	//    // RENDER EDGES
	//    for (u32 i = 0; i < face_normals.size(); i++)
	//    {
	//       u32 f = i * 3;
	//       ImDraw::add_line(IMHASH, polytope[faces[f    ]], polytope[faces[f + 1]], 1.5, true, vec3(0.3, 0.5, 0.2));
	//       ImDraw::add_line(IMHASH, polytope[faces[f + 1]], polytope[faces[f + 2]], 1.5, true, vec3(0.3, 0.5, 0.2));
	//       ImDraw::add_line(IMHASH, polytope[faces[f + 2]], polytope[faces[f    ]], 1.5, true, vec3(0.3, 0.5, 0.2));
	//    }

	//    // RENDER ORIGIN
	//    ImDraw::add_point(IMHASH, vec3(0), 3.0, false, vec3(0.956, 0.784, 0.184));

	//    // RENDER PENETRATION VECTOR
	//    ImDraw::add_line(IMHASH, vec3(0), penetration_normal * min_distance_to_face, 2.0, false, vec3(0.882, 0.254, 0.878));
	// }

	EpaResult Result;

	if (MinDistanceToFace != MaxFloat)
		Result.Collision = true;

	// if (min_distance_to_face != MAX_FLOAT)
	// assert(false);

	Result.Direction = PenetrationNormal;
	Result.Penetration = MinDistanceToFace + 0.0001f;

	return Result;
}
