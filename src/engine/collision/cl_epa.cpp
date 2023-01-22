#include <vector>
#include <string>
#include <map>
#include <engine/core/types.h>
#include <engine/collision/primitives/bounding_box.h>
#include <engine/collision/primitives/ray.h>
#include <engine/vertex.h>
#include <engine/mesh.h>
#include <colors.h>
#include <engine/render/renderer.h>
#include <engine/render/im_render.h>
#include <engine/collision/simplex.h>
#include <engine/collision/cl_gjk.h>
#include <engine/collision/cl_epa.h>
#include <chrono>
#include <iostream>

extern const int ClMaxEpaIterations = 100;

std::pair<std::vector<vec4>, size_t> CL_EPA_get_face_normals_and_closest_face(
const std::vector<vec3>& polytope,
const std::vector<size_t>& faces)
{
	std::vector<vec4> normals;
	size_t closest_face_index = 0;
	float min_distance_to_face = MaxFloat;

	for(size_t i = 0; i < faces.size(); i += 3)
	{
		vec3 a = polytope[faces[i]];
		vec3 b = polytope[faces[i + 1]];
		vec3 c = polytope[faces[i + 2]];

		vec3 normal = normalize(glm::cross(b - a, c - a));
		float distance = dot(normal, c);

		if(distance < 0)
		{
			normal *= -1;
			distance *= -1;
		}

		normals.emplace_back(normal, distance);

		if(distance < min_distance_to_face)
		{
			closest_face_index = i / 3;
			min_distance_to_face = distance;
		}
	}

	return {normals, closest_face_index};
}


void CL_add_if_outer_edge(
std::vector<std::pair<size_t, size_t> >& edges,
const std::vector<size_t>& faces,
size_t a,
size_t b)
{
	// if edge is already in list (but in reverse winding order)
	// then we must exclude it from the list as it is not an outer edge.
	// if we don't find it, just add.

	auto reverse = std::find(          //      0--<--3
	edges.begin(),                     //     / \ B /   A: 2-0
	edges.end(),                       //    / A \ /    B: 0-2
	std::make_pair(faces[b], faces[a]) //   1-->--2
	);

	if(reverse != edges.end())
		edges.erase(reverse);
	else
		edges.emplace_back(faces[a], faces[b]);
}

static void get_time(int elapsed)
{
	static std::vector<int> times;
	const int N = 100;

	times.push_back(elapsed);


	if(times.size() == N)
	{
		int sum = 0;
		for(int i = 0; i < times.size(); i++)
			sum += times[i];

		float average = sum * 1.0 / N;

		std::cout << "Average time spent on EPA: " << average << "\n";

		times.clear();
	}
}


EPA_Result CL_run_EPA(Simplex simplex, CollisionMesh* collider_a, CollisionMesh* collider_b)
{
	using micro = std::chrono::microseconds;
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<vec3> polytope;
	polytope.insert(polytope.begin(), std::begin(simplex.points), std::end(simplex.points));

	std::vector<size_t> faces = {
	0, 1, 2,
	0, 3, 1,
	0, 2, 3,
	1, 3, 2
	};

	auto [face_normals, closest_face_index] = CL_EPA_get_face_normals_and_closest_face(polytope, faces);

	vec3 penetration_normal;
	float min_distance_to_face = MaxFloat;

	int EPA_iterations = 0;

	while(min_distance_to_face == MaxFloat && EPA_iterations < ClMaxEpaIterations)
	{
		EPA_iterations++;
		penetration_normal = vec3(face_normals[closest_face_index]);
		min_distance_to_face = face_normals[closest_face_index].w;

		GJK_Point support = CL_get_support_point(collider_a, collider_b, penetration_normal);
		if(support.empty || CL_support_is_in_polytope(polytope, support.point))
			break;

		float s_distance = dot(penetration_normal, support.point);

		// if we have a vertex in support direction and its farther enough to check
		if(abs(s_distance - min_distance_to_face) > 0.001f)
		{
			min_distance_to_face = MaxFloat;

			// removes all faces pointing towards the support direction and lists the outer_edges
			std::vector<std::pair<size_t, size_t> > outer_edges;
			for(size_t i = 0; i < face_normals.size(); i++)
			{
				if(CL_same_general_direction(face_normals[i], support.point))
				{
					size_t f = i * 3;

					CL_add_if_outer_edge(outer_edges, faces, f, f + 1);
					CL_add_if_outer_edge(outer_edges, faces, f + 1, f + 2);
					CL_add_if_outer_edge(outer_edges, faces, f + 2, f);

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
			std::vector<size_t> new_faces;
			for(auto [edgeIndex1, edgeIndex2] : outer_edges)
			{
				new_faces.push_back(edgeIndex1);
				new_faces.push_back(edgeIndex2);
				new_faces.push_back(polytope.size());
			}

			polytope.push_back(support.point);

			// finds normals and closest face from the new faces
			auto [new_normals, new_closest_face_index] = CL_EPA_get_face_normals_and_closest_face(polytope, new_faces);

			float old_min_distance_to_face = MaxFloat;
			for(size_t i = 0; i < face_normals.size(); i++)
			{
				float dist = face_normals[i].w;
				if(dist < old_min_distance_to_face)
				{
					old_min_distance_to_face = dist;
					closest_face_index = i;
				}
			}

			if(new_normals[new_closest_face_index].w < old_min_distance_to_face)
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
	//    for (size_t i = 0; i < face_normals.size(); i++)
	//    {
	//       size_t f = i * 3;
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

	if(min_distance_to_face != MaxFloat)
		result.collision = true;

	// if (min_distance_to_face != MAX_FLOAT)
	// assert(false);

	result.direction = penetration_normal;
	result.penetration = min_distance_to_face + 0.0001f;

	auto finish = std::chrono::high_resolution_clock::now();
	int elapsed = std::chrono::duration_cast<micro>(finish - start).count();
	// get_time(elapsed);

	return result;
}
