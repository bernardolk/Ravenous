#include <engine/core/types.h>
#include <engine/collision/simplex.h>
#include <engine/collision/ClGjk.h>
#include <engine/collision/ClEpa.h>

extern const int ClMaxEpaIterations = 100;

std::pair<std::vector<vec4>, u32> CL_GetEPAFaceNormalsAndClosestFace(
	const std::vector<vec3>& polytope,
	const std::vector<u32>& faces)
{
	std::vector<vec4> normals;
	u32 closest_face_index = 0;
	float min_distance_to_face = MaxFloat;

	for (u32 i = 0; i < faces.size(); i += 3)
	{
		vec3 a = polytope[faces[i]];
		vec3 b = polytope[faces[i + 1]];
		vec3 c = polytope[faces[i + 2]];

		vec3 normal = normalize(glm::cross(b - a, c - a));
		float distance = dot(normal, c);

		if (distance < 0)
		{
			normal *= -1;
			distance *= -1;
		}

		normals.emplace_back(normal, distance);

		if (distance < min_distance_to_face)
		{
			closest_face_index = i / 3;
			min_distance_to_face = distance;
		}
	}

	return {normals, closest_face_index};
}


void CL_AddIfOuterEdge(
	std::vector<std::pair<u32, u32> >& edges,
	const std::vector<u32>& faces,
	u32 a,
	u32 b)
{
	// if edge is already in list (but in reverse winding order)
	// then we must exclude it from the list as it is not an outer edge.
	// if we don't find it, just add.

	auto reverse = std::find(              //      0--<--3
		edges.begin(),                     //     / \ B /   A: 2-0
		edges.end(),                       //    / A \ /    B: 0-2
		std::make_pair(faces[b], faces[a]) //   1-->--2
	);

	if (reverse != edges.end())
		edges.erase(reverse);
	else
		edges.emplace_back(faces[a], faces[b]);
}

EPA_Result CL_RunEPA(Simplex simplex, CollisionMesh* collider_a, CollisionMesh* collider_b)
{

	std::vector<vec3> polytope;
	polytope.insert(polytope.begin(), std::begin(simplex.points), std::end(simplex.points));

	std::vector<u32> faces = {
	0, 1, 2,
	0, 3, 1,
	0, 2, 3,
	1, 3, 2
	};

	auto [face_normals, closest_face_index] = CL_GetEPAFaceNormalsAndClosestFace(polytope, faces);

	vec3 penetration_normal;
	float min_distance_to_face = MaxFloat;

	int EPA_iterations = 0;

	while (min_distance_to_face == MaxFloat && EPA_iterations < ClMaxEpaIterations)
	{
		EPA_iterations++;
		penetration_normal = vec3(face_normals[closest_face_index]);
		min_distance_to_face = face_normals[closest_face_index].w;

		GJK_Point support = CL_GetSupportPoint(collider_a, collider_b, penetration_normal);
		if (support.empty || CL_SupportIsInPolytope(polytope, support.point))
			break;

		float s_distance = dot(penetration_normal, support.point);

		// if we have a vertex in support direction and its farther enough to check
		if (abs(s_distance - min_distance_to_face) > 0.001f)
		{
			min_distance_to_face = MaxFloat;

			// removes all faces pointing towards the support direction and lists the outer_edges
			std::vector<std::pair<u32, u32> > outer_edges;
			for (u32 i = 0; i < face_normals.size(); i++)
			{
				if (CL_SameGeneralDirection(face_normals[i], support.point))
				{
					u32 f = i * 3;

					CL_AddIfOuterEdge(outer_edges, faces, f, f + 1);
					CL_AddIfOuterEdge(outer_edges, faces, f + 1, f + 2);
					CL_AddIfOuterEdge(outer_edges, faces, f + 2, f);

					faces[f + 2] = faces.back();
					faces.pop_back();
					faces[f + 1] = faces.back();
					faces.pop_back();
					faces[f] = faces.back();
					faces.pop_back();

					face_normals[i] = face_normals.back();
					face_normals.pop_back();

					i--;
				}
			}

			// construct new faces from the outer_edges listed before
			std::vector<u32> new_faces;
			for (auto [edgeIndex1, edgeIndex2] : outer_edges)
			{
				new_faces.push_back(edgeIndex1);
				new_faces.push_back(edgeIndex2);
				new_faces.push_back(polytope.size());
			}

			polytope.push_back(support.point);

			// finds normals and closest face from the new faces
			auto [new_normals, new_closest_face_index] = CL_GetEPAFaceNormalsAndClosestFace(polytope, new_faces);

			float old_min_distance_to_face = MaxFloat;
			for (u32 i = 0; i < face_normals.size(); i++)
			{
				float dist = face_normals[i].w;
				if (dist < old_min_distance_to_face)
				{
					old_min_distance_to_face = dist;
					closest_face_index = i;
				}
			}

			if (new_normals[new_closest_face_index].w < old_min_distance_to_face)
				closest_face_index = new_closest_face_index + face_normals.size();

			faces.insert(faces.end(), new_faces.begin(), new_faces.end());
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

	EPA_Result result;

	if (min_distance_to_face != MaxFloat)
		result.collision = true;

	// if (min_distance_to_face != MAX_FLOAT)
	// assert(false);

	result.direction = penetration_normal;
	result.penetration = min_distance_to_face + 0.0001f;

	return result;
}
