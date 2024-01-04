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

	float Maxx = 0.f, Minx = 0.f, Maxy = 0.f, Miny = 0.f, Maxz = 0.f, Minz = 0.f;

	for (auto& Vertex : Vertices)
	{
		if (Vertex.x < Minx) Minx = Vertex.x;
		if (Vertex.x > Maxx) Maxx = Vertex.x;
		if (Vertex.y < Miny) Miny = Vertex.y;
		if (Vertex.y > Maxy) Maxy = Vertex.y;
		if (Vertex.z < Minz) Minz = Vertex.z;
		if (Vertex.z > Maxz) Maxz = Vertex.z;
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
