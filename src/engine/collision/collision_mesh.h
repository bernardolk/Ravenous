#pragma once

struct BoundingBox;
struct Mesh;

struct CollisionMesh
{
	std::vector<vec3> vertices;
	std::vector<u32> indices;

	BoundingBox compute_bounding_box();
};

// CollisionMesh* cmesh_from_mesh(Mesh* mesh);
