#pragma once

struct RBoundingBox;
struct RMesh;

struct RCollisionMesh
{
	std::vector<vec3> vertices;
	std::vector<u32> indices;

	RBoundingBox ComputeBoundingBox();
};

// CollisionMesh* cmesh_from_mesh(Mesh* mesh);
