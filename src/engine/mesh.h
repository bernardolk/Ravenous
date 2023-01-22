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
	std::vector<Vertex> vertices;
	std::vector<u32> indices;
	u32 faces_count;
	u32 render_method;
	GLData gl_data;
	std::string name;
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
	std::string type;
	std::string path;
	std::string name;
};

extern std::map<std::string, Texture> TextureCatalogue;
extern std::map<std::string, Mesh*> GeometryCatalogue;
extern std::map<std::string, CollisionMesh*> CollisionGeometryCatalogue;


GLData setup_gl_data_for_lines(const Vertex* vertices, size_t size);
std::vector<Vertex> construct_cylinder(float radius, float half_lenght, int slices);
Triangle get_triangle_for_collider_indexed_mesh(const Mesh* mesh, int triangle_index);
Triangle get_triangle_for_collider_indexed_mesh(const CollisionMesh* mesh, int triangle_index);
Triangle get_triangle_for_indexed_mesh(Mesh* mesh, glm::mat4 mat_model, int triangle_index);
