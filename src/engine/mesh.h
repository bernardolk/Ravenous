#pragma once

#include <map>

struct Vertex;
struct BoundingBox;
struct Triangle;
struct CollisionMesh;

struct GLData
{
	u32 VAO = 0;
	u32 VBO = 0;
	u32 EBO = 0;
};

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<u32>    indices;
	u32                 faces_count;
	u32                 render_method;
	GLData              gl_data;
	std::string         name;
	//  FILETIME               last_written;

	void        setup_gl_data();
	void        setup_gl_buffers();
	void        send_data_to_gl_buffer();
	void        compute_tangents_and_bitangents();
	BoundingBox compute_bounding_box();
};

struct Texture
{
	unsigned int id;
	std::string  type;
	std::string  path;
	std::string  name;
};

extern std::map<std::string, Texture>        Texture_Catalogue;
extern std::map<std::string, Mesh*>          Geometry_Catalogue;
extern std::map<std::string, CollisionMesh*> Collision_Geometry_Catalogue;


GLData              setup_gl_data_for_lines(Vertex* vertices, size_t size);
std::vector<Vertex> construct_cylinder(float radius, float half_lenght, int slices);
Triangle            get_triangle_for_collider_indexed_mesh(Mesh* mesh, int triangle_index);
Triangle            get_triangle_for_collider_indexed_mesh(CollisionMesh* mesh, int triangle_index);
Triangle            get_triangle_for_indexed_mesh(Mesh* mesh, glm::mat4 matModel, int triangle_index);
