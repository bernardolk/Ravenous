#pragma once

struct RBoundingBox;
struct RMesh;

struct RCollisionMesh
{
	std::vector<vec3> vertices;
	std::vector<uint> indices;

	RBoundingBox ComputeBoundingBox();
};

// CollisionMesh* cmesh_from_mesh(Mesh* mesh);
