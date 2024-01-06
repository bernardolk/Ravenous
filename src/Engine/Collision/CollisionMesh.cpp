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

	RBoundingBox Box;
	for (auto& Vertex : Vertices)
	{
		if (Vertex.x < Box.MinX) Box.MinX = Vertex.x;
		if (Vertex.x > Box.MaxX) Box.MaxX = Vertex.x;
		if (Vertex.y < Box.MinY) Box.MinY = Vertex.y;
		if (Vertex.y > Box.MaxY) Box.MaxY = Vertex.y;
		if (Vertex.z < Box.MinZ) Box.MinZ = Vertex.z;
		if (Vertex.z > Box.MaxZ) Box.MaxZ = Vertex.z;
	}
	
	return Box;
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
