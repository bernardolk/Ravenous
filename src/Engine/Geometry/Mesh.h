#pragma once

#include "engine/core/core.h"
#include "vertex.h"

struct GLData
{
	u32 VAO = 0;
	u32 VBO = 0;
	u32 EBO = 0;
};

struct Mesh
{
	vector<Vertex> vertices;
	vector<u32> indices;
	u32 faces_count;
	u32 render_method;
	GLData gl_data;
	string name;
	//  FILETIME               last_written;

	void SetupGLData();
	void SetupGLBuffers();
	void SendDataToGLBuffer();
	void ComputeTangentsAndBitangents();
	BoundingBox ComputeBoundingBox();
};

struct Texture
{
	unsigned int id;
	string type;
	string path;
	string name;
};

extern map<string, Texture> TextureCatalogue;
extern map<string, Mesh*> GeometryCatalogue;
extern map<string, CollisionMesh*> CollisionGeometryCatalogue;


GLData setup_gl_data_for_lines(const Vertex* vertices, u32 size);
vector<Vertex> construct_cylinder(float radius, float half_lenght, int slices);
Triangle get_triangle_for_collider_indexed_mesh(const Mesh* mesh, int triangle_index);
Triangle get_triangle_for_collider_indexed_mesh(const CollisionMesh* mesh, int triangle_index);
Triangle get_triangle_for_indexed_mesh(Mesh* mesh, glm::mat4 mat_model, int triangle_index);
