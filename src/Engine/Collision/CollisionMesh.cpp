#include <vector>
#include "Engine/Core/Types.h"
#include "Engine/Collision/Primitives/BoundingBox.h"
#include "Engine/Collision/CollisionMesh.h"


RBoundingBox RCollisionMesh::ComputeBoundingBox()
{
	// This returns a bounding box that contains the mesh
	// Vertices of the bounding box do not necessarely match vertices in the mesh
	// So, this does NOT return the min/max vertices of the mesh in axial direction
	// (support points)

	auto MaxD = vec3(MinFloat, MinFloat, MinFloat);
	auto MinD = vec3(MaxFloat, MaxFloat, MaxFloat);

	float Maxx = 0.f, Minx = 0.f, Maxy = 0.f, Miny = 0.f, Maxz = 0.f, Minz = 0.f;

	for (int i = 0; i < this->Vertices.size(); i++)
	{
		vec3 Vertex = this->Vertices[i];
		float Dotx = dot(Vertex, vec3(1, 0, 0));
		float Doty = dot(Vertex, vec3(0, 1, 0));
		float Dotz = dot(Vertex, vec3(0, 0, 1));

		if (Dotx < MinD.x)
		{
			Minx = Vertex.x;
			MinD.x = Dotx;
		}
		if (Dotx > MaxD.x)
		{
			Maxx = Vertex.x;
			MaxD.x = Dotx;
		}

		if (Doty < MinD.y)
		{
			Miny = Vertex.y;
			MinD.y = Doty;
		}
		if (Doty > MaxD.y)
		{
			Maxy = Vertex.y;
			MaxD.y = Doty;
		}

		if (Dotz < MinD.z)
		{
			Minz = Vertex.z;
			MinD.z = Dotz;
		}
		if (Dotz > MaxD.z)
		{
			Maxz = Vertex.z;
			MaxD.z = Dotz;
		}
	}

	RBoundingBox Bb;
	Bb.Set(vec3(Minx, Miny, Minz), vec3(Maxx, Maxy, Maxz));
	return Bb;
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
