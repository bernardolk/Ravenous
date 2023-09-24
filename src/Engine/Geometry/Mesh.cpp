#include <engine/core/core.h>
#ifndef GLAD_INCL
#define GLAD_INCL
#include <glad/glad.h>
#endif
#include <engine/collision/primitives/BoundingBox.h>
#include <glm/gtx/normal.hpp>
#include "engine/geometry/triangle.h"
#include "engine/geometry/vertex.h"
#include <iostream>
#include "engine/core/logging.h"
#include "engine/geometry/mesh.h"
#include <engine/collision/CollisionMesh.h>

/*
   Mesh vertex readme:
   - Vertex count = N_quad_faces * 4
   - Indices count = N_quad_faces * 6 or N_triang_faces * 3
*/


map<std::string, RMesh*> GeometryCatalogue;
map<std::string, RCollisionMesh*> CollisionGeometryCatalogue;
map<std::string, RTexture> TextureCatalogue;

void RMesh::SetupGLData()
{
	// to avoid a pretty bad rendering issue
	assert(indices.size() > 0);

	if (gl_data.VAO > 0)
	{
		Log(LOG_INFO, "Redundant setup_gl_data call occured.");
		return;
	}

	RGLData new_gl_data;

	// create buffers/arrays
	glGenVertexArrays(1, &new_gl_data.VAO);
	glGenBuffers(1, &new_gl_data.VBO);
	glGenBuffers(1, &new_gl_data.EBO);

	// load data into vertex buffers
	glBindVertexArray(new_gl_data.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, new_gl_data.VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(RVertex), &(vertices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_gl_data.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &(indices[0]), GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), static_cast<void*>(nullptr));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, tex_coords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, bitangent));

	glBindVertexArray(0);

	gl_data = new_gl_data;
}

// This will only create the buffer and set the attribute pointers
void RMesh::SetupGLBuffers()
{
	glGenVertexArrays(1, &this->gl_data.VAO);
	glGenBuffers(1, &this->gl_data.VBO);
	glGenBuffers(1, &this->gl_data.EBO);
}

void RMesh::SendDataToGLBuffer()
{
	// load data into vertex buffers
	glBindVertexArray(this->gl_data.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->gl_data.VBO);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(RVertex), &(this->vertices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->gl_data.EBO);

	if (this->indices.size() > 0)
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &(this->indices[0]), GL_STATIC_DRAW);
	}
	// @TODO: Do we need to do this every time?
	// set the vertex attribute pointers
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), static_cast<void*>(nullptr));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, tex_coords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, bitangent));

	glBindVertexArray(0);
}


RBoundingBox RMesh::ComputeBoundingBox()
{
	// This returns a bounding box that contains the mesh
	// Vertices of the bounding box do not necessarily match vertices in the mesh
	// So, this does NOT return the min/max vertices of the mesh in axial direction
	// (support points)

	auto max_d = vec3(MinFloat, MinFloat, MinFloat);
	auto min_d = vec3(MaxFloat, MaxFloat, MaxFloat);

	float maxx = 0.f, minx = 0.f, maxy = 0.f, miny = 0.f, maxz = 0.f, minz = 0.f;

	for (int i = 0; i < this->vertices.size(); i++)
	{
		vec3 vertex = this->vertices[i].position;
		float dotx = dot(vertex, vec3(1, 0, 0));
		float doty = dot(vertex, vec3(0, 1, 0));
		float dotz = dot(vertex, vec3(0, 0, 1));

		if (dotx < min_d.x)
		{
			minx = vertex.x;
			min_d.x = dotx;
		}
		if (dotx > max_d.x)
		{
			maxx = vertex.x;
			max_d.x = dotx;
		}

		if (doty < min_d.y)
		{
			miny = vertex.y;
			min_d.y = doty;
		}
		if (doty > max_d.y)
		{
			maxy = vertex.y;
			max_d.y = doty;
		}

		if (dotz < min_d.z)
		{
			minz = vertex.z;
			min_d.z = dotz;
		}
		if (dotz > max_d.z)
		{
			maxz = vertex.z;
			max_d.z = dotz;
		}
	}

	RBoundingBox bb{};
	bb.Set(vec3(minx, miny, minz), vec3(maxx, maxy, maxz));
	return bb;
}

void RMesh::ComputeTangentsAndBitangents()
{
	// @TODO: This may lead to bugs, we currentyl assume here that faces = 2 triangles each, and while that may hold true with the current loader, that may not remain the case forever.
	For(this->faces_count)
	{
		RVertex v1 = this->vertices[indices[i * 3 + 0]];
		RVertex v2 = this->vertices[indices[i * 3 + 1]];
		RVertex v3 = this->vertices[indices[i * 3 + 2]];

		vec3 edge1 = v2.position - v1.position;
		vec3 edge2 = v3.position - v1.position;
		vec2 delta_uv1 = v2.tex_coords - v1.tex_coords;
		vec2 delta_uv2 = v3.tex_coords - v1.tex_coords;

		float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

		vec3 tangent, bitangent;
		tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
		tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
		tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);

		bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
		bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
		bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);

		this->vertices[indices[i * 3 + 0]].tangent = tangent;
		this->vertices[indices[i * 3 + 0]].bitangent = bitangent;

		this->vertices[indices[i * 3 + 1]].tangent = tangent;
		this->vertices[indices[i * 3 + 1]].bitangent = bitangent;

		this->vertices[indices[i * 3 + 2]].tangent = tangent;
		this->vertices[indices[i * 3 + 2]].bitangent = bitangent;
	}
}

RGLData setup_gl_data_for_lines(const RVertex* vertices, u32 size)
{
	RGLData gl_data;

	// create buffers/arrays
	glGenVertexArrays(1, &gl_data.VAO);
	glGenBuffers(1, &gl_data.VBO);

	// load data into vertex buffers
	glBindVertexArray(gl_data.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, gl_data.VBO);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(RVertex), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), static_cast<void*>(nullptr));

	return gl_data;
}

std::vector<RVertex> construct_cylinder(float radius, float half_lenght, int slices)
{
	std::vector<RVertex> vertices;
	for (int i = 0; i < slices; i++)
	{
		float theta = static_cast<float>(i) * 2.0 * PI * (1.0 / slices);
		float next_theta = (static_cast<float>(i) + 1) * 2.0 * PI * (1.0 / slices);
		// vertex at middle of end  
		auto v = RVertex{vec3(0.0), vec3(half_lenght), vec3(0.0)};
		vertices.push_back(v);
		//vertices at edges of circle 
		v = RVertex{vec3(radius * cos(theta), half_lenght, radius * sin(theta))};
		vertices.push_back(v);
		v = RVertex{vec3(radius * cos(next_theta), half_lenght, radius * sin(next_theta))};
		vertices.push_back(v);
		// the same vertices at the bottom of the cylinder (half face)
		v = RVertex{vec3(radius * cos(next_theta), -half_lenght, radius * sin(next_theta))};
		vertices.push_back(v);
		v = RVertex{vec3(radius * cos(theta), -half_lenght, radius * sin(theta))};
		vertices.push_back(v);
		// other half face
		v = RVertex{vec3(radius * cos(theta), half_lenght, radius * sin(theta))};
		vertices.push_back(v);
		v = RVertex{vec3(radius * cos(next_theta), half_lenght, radius * sin(next_theta))};
		vertices.push_back(v);
		// back from the middle
		v = RVertex{vec3(radius * cos(next_theta), -half_lenght, radius * sin(next_theta))};
		vertices.push_back(v);
		v = RVertex{vec3(0.0, -half_lenght, 0.0)};
		vertices.push_back(v);
		// roundabout
		v = RVertex{vec3(radius * cos(theta), -half_lenght, radius * sin(theta))};
		vertices.push_back(v);
		v = RVertex{vec3(radius * cos(next_theta), -half_lenght, radius * sin(next_theta))};
		vertices.push_back(v);
		v = RVertex{vec3(0.0, -half_lenght, 0.0)};
		vertices.push_back(v);
	}

	return vertices;
}

// -----------------------------------------
// > GET TRIANGLE FOR COLLIDER INDEXED MESH
// -----------------------------------------
RTriangle get_triangle_for_collider_indexed_mesh(const RMesh* mesh, int triangle_index)
{
	auto a_ind = mesh->indices[3 * triangle_index + 0];
	auto b_ind = mesh->indices[3 * triangle_index + 1];
	auto c_ind = mesh->indices[3 * triangle_index + 2];

	auto a = mesh->vertices[a_ind].position;
	auto b = mesh->vertices[b_ind].position;
	auto c = mesh->vertices[c_ind].position;

	return RTriangle{a, b, c};
}

RTriangle get_triangle_for_collider_indexed_mesh(const RCollisionMesh* mesh, int triangle_index)
{
	auto a_ind = mesh->indices[3 * triangle_index + 0];
	auto b_ind = mesh->indices[3 * triangle_index + 1];
	auto c_ind = mesh->indices[3 * triangle_index + 2];

	auto a = mesh->vertices[a_ind];
	auto b = mesh->vertices[b_ind];
	auto c = mesh->vertices[c_ind];

	return RTriangle{a, b, c};
}

// --------------------------------
// > GET TRIANGLE FOR INDEXED MESH
// --------------------------------
RTriangle get_triangle_for_indexed_mesh(RMesh* mesh, glm::mat4 mat_model, int triangle_index)
{
	auto a_ind = mesh->indices[3 * triangle_index + 0];
	auto b_ind = mesh->indices[3 * triangle_index + 1];
	auto c_ind = mesh->indices[3 * triangle_index + 2];

	auto a_vertice = mesh->vertices[a_ind].position;
	auto b_vertice = mesh->vertices[b_ind].position;
	auto c_vertice = mesh->vertices[c_ind].position;

	auto a = mat_model * glm::vec4(a_vertice, 1.0);
	auto b = mat_model * glm::vec4(b_vertice, 1.0);
	auto c = mat_model * glm::vec4(c_vertice, 1.0);

	return RTriangle{a, b, c};
}
