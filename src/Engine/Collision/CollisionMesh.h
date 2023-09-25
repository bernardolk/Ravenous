#pragma once

struct RBoundingBox;
struct RMesh;

struct RCollisionMesh
{
	vector<vec3> Vertices;
	vector<uint> Indices;

	RBoundingBox ComputeBoundingBox();
};

// CollisionMesh* cmesh_from_mesh(Mesh* mesh);
