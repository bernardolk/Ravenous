#pragma once

struct RBoundingBox;
struct RMesh;

struct RCollisionMesh
{
	vector<vec3> Vertices;
	vector<uint> Indices;

	RBoundingBox ComputeBoundingBox();

	RCollisionMesh(){}
	RCollisionMesh(const RCollisionMesh& Other)
	{
		Vertices = Other.Vertices;
		Indices = Other.Indices;		
	}
	RCollisionMesh& operator=(const RCollisionMesh& Other)
	{
		Vertices = Other.Vertices;
		Indices = Other.Indices;
		return *this;
	}
	~RCollisionMesh()
	{
		int a = 0;
		static_assert(true);
	}
};

// CollisionMesh* cmesh_from_mesh(Mesh* mesh);
