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
	vector<RVertex> vertices;
	vector<uint> indices;
	uint faces_count;
	uint render_method;
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


RGLData setup_gl_data_for_lines(const RVertex* vertices, uint size);
vector<RVertex> construct_cylinder(float radius, float half_lenght, int slices);
RTriangle get_triangle_for_collider_indexed_mesh(const RMesh* mesh, int triangle_index);
RTriangle get_triangle_for_collider_indexed_mesh(const RCollisionMesh* mesh, int triangle_index);
RTriangle get_triangle_for_indexed_mesh(RMesh* mesh, glm::mat4 mat_model, int triangle_index);
