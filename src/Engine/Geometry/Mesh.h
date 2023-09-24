#pragma once

#include "engine/core/core.h"
#include "vertex.h"

struct RGLData
{
	u32 VAO = 0;
	u32 VBO = 0;
	u32 EBO = 0;
};

struct RMesh
{
	vector<RVertex> vertices;
	vector<u32> indices;
	u32 faces_count;
	u32 render_method;
	RGLData gl_data;
	string name;
	//  FILETIME               last_written;

	void SetupGLData();
	void SetupGLBuffers();
	void SendDataToGLBuffer();
	void ComputeTangentsAndBitangents();
	RBoundingBox ComputeBoundingBox();
};

struct RTexture
{
	unsigned int id;
	string type;
	string path;
	string name;
};

extern map<string, RTexture> TextureCatalogue;
extern map<string, RMesh*> GeometryCatalogue;
extern map<string, RCollisionMesh*> CollisionGeometryCatalogue;


RGLData setup_gl_data_for_lines(const RVertex* vertices, u32 size);
vector<RVertex> construct_cylinder(float radius, float half_lenght, int slices);
RTriangle get_triangle_for_collider_indexed_mesh(const RMesh* mesh, int triangle_index);
RTriangle get_triangle_for_collider_indexed_mesh(const RCollisionMesh* mesh, int triangle_index);
RTriangle get_triangle_for_indexed_mesh(RMesh* mesh, glm::mat4 mat_model, int triangle_index);
