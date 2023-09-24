#include <string>
#include <vector>
#include <map>
#include <engine/core/types.h>
#include <engine/collision/primitives/BoundingBox.h>
#include "engine/geometry/mesh.h"
#include "engine/geometry/vertex.h"
#include <engine/collision/CollisionMesh.h>


RBoundingBox RCollisionMesh::ComputeBoundingBox()
{
	// This returns a bounding box that contains the mesh
	// Vertices of the bounding box do not necessarely match vertices in the mesh
	// So, this does NOT return the min/max vertices of the mesh in axial direction
	// (support points)

	auto max_d = vec3(MinFloat, MinFloat, MinFloat);
	auto min_d = vec3(MaxFloat, MaxFloat, MaxFloat);

	float maxx = 0.f, minx = 0.f, maxy = 0.f, miny = 0.f, maxz = 0.f, minz = 0.f;

	for (int i = 0; i < this->vertices.size(); i++)
	{
		vec3 vertex = this->vertices[i];
		float dotx = dot(vertex, vec3(1, 0, 0));
		float doty = dot(vertex, vec3(0, 1, 0));
		float dotz = dot(vertex, vec3(0, 0, 1));

		if (dotx < min_d.x)
		{
			minx = vertex.x;
			min_d.x = dotx;
		}
		if (dotx > max_d.x)
		{
			maxx = vertex.x;
			max_d.x = dotx;
		}

		if (doty < min_d.y)
		{
			miny = vertex.y;
			min_d.y = doty;
		}
		if (doty > max_d.y)
		{
			maxy = vertex.y;
			max_d.y = doty;
		}

		if (dotz < min_d.z)
		{
			minz = vertex.z;
			min_d.z = dotz;
		}
		if (dotz > max_d.z)
		{
			maxz = vertex.z;
			max_d.z = dotz;
		}
	}

	RBoundingBox bb;
	bb.Set(vec3(minx, miny, minz), vec3(maxx, maxy, maxz));
	return bb;
}


// CollisionMesh* cmesh_from_mesh(Mesh* mesh)
// {
//    auto c_mesh = new CollisionMesh();
//    // copy positions
//    auto& mesh_v = mesh->vertices;
//    For(mesh_v.size())
//       c_mesh->vertices.push_back(mesh_v[i].position);
//    // copy
//    auto& mesh_i = mesh->indices;
//    For(mesh_i.size())
//       c_mesh->indices.push_back(mesh_i[i]);

//    return c_mesh;
// }
