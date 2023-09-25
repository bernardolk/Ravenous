#pragma once

#include "engine/core/core.h"
#include "vertex.h"

struct RGLData
{
	uint VAO = 0;
	uint VBO = 0;
	uint EBO = 0;
};

struct RMesh
{
	vector<RVertex> Vertices;
	vector<uint> Indices;
	uint FacesCount;
	uint RenderMethod;
	RGLData GLData;
	string Name;
	//  FILETIME               last_written;

	void SetupGLData();
	void SetupGLBuffers();
	void SendDataToGLBuffer();
	void ComputeTangentsAndBitangents();
	RBoundingBox ComputeBoundingBox();
};

struct RTexture
{
	unsigned int ID;
	string Type;
	string Path;
	string Name;
};

extern map<string, RTexture> TextureCatalogue;
extern map<string, RMesh*> GeometryCatalogue;
extern map<string, RCollisionMesh*> CollisionGeometryCatalogue;


RGLData setup_gl_data_for_lines(const RVertex* Vertices, uint Size);
vector<RVertex> construct_cylinder(float Radius, float HalfLenght, int Slices);
RTriangle get_triangle_for_collider_indexed_mesh(const RMesh* Mesh, int TriangleIndex);
RTriangle get_triangle_for_collider_indexed_mesh(const RCollisionMesh* Mesh, int TriangleIndex);
RTriangle get_triangle_for_indexed_mesh(RMesh* Mesh, glm::mat4 MatModel, int TriangleIndex);
