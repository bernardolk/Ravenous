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
	assert(Indices.size() > 0);

	if (GLData.VAO > 0)
	{
		Log(LOG_INFO, "Redundant setup_gl_data call occured.");
		return;
	}

	RGLData NewGlData;

	// create buffers/arrays
	glGenVertexArrays(1, &NewGlData.VAO);
	glGenBuffers(1, &NewGlData.VBO);
	glGenBuffers(1, &NewGlData.EBO);

	// load data into vertex buffers
	glBindVertexArray(NewGlData.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, NewGlData.VBO);
	glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(RVertex), &(Vertices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewGlData.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(uint), &(Indices[0]), GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), static_cast<void*>(nullptr));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, Bitangent));

	glBindVertexArray(0);

	GLData = NewGlData;
}

// This will only create the buffer and set the attribute pointers
void RMesh::SetupGLBuffers()
{
	glGenVertexArrays(1, &this->GLData.VAO);
	glGenBuffers(1, &this->GLData.VBO);
	glGenBuffers(1, &this->GLData.EBO);
}

void RMesh::SendDataToGLBuffer()
{
	// load data into vertex buffers
	glBindVertexArray(this->GLData.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->GLData.VBO);
	glBufferData(GL_ARRAY_BUFFER, this->Vertices.size() * sizeof(RVertex), &(this->Vertices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->GLData.EBO);

	if (this->Indices.size() > 0)
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->Indices.size() * sizeof(unsigned int), &(this->Indices[0]), GL_STATIC_DRAW);
	}
	// @TODO: Do we need to do this every time?
	// set the vertex attribute pointers
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), static_cast<void*>(nullptr));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), (void*)offsetof(RVertex, Bitangent));

	glBindVertexArray(0);
}


RBoundingBox RMesh::ComputeBoundingBox()
{
	// This returns a bounding box that contains the mesh
	// Vertices of the bounding box do not necessarily match vertices in the mesh
	// So, this does NOT return the min/max vertices of the mesh in axial direction
	// (support points)

	auto MaxD = vec3(MinFloat, MinFloat, MinFloat);
	auto MinD = vec3(MaxFloat, MaxFloat, MaxFloat);

	float Maxx = 0.f, Minx = 0.f, Maxy = 0.f, Miny = 0.f, Maxz = 0.f, Minz = 0.f;

	for (int i = 0; i < this->Vertices.size(); i++)
	{
		vec3 Vertex = this->Vertices[i].Position;
		float Dotx = dot(Vertex, vec3(1, 0, 0));
		float Doty = dot(Vertex, vec3(0, 1, 0));
		float Dotz = dot(Vertex, vec3(0, 0, 1));

		if (Dotx < MinD.x)
		{
			Minx = Vertex.x;
			MinD.x = Dotx;
		}
		if (Dotx > MaxD.x)
		{
			Maxx = Vertex.x;
			MaxD.x = Dotx;
		}

		if (Doty < MinD.y)
		{
			Miny = Vertex.y;
			MinD.y = Doty;
		}
		if (Doty > MaxD.y)
		{
			Maxy = Vertex.y;
			MaxD.y = Doty;
		}

		if (Dotz < MinD.z)
		{
			Minz = Vertex.z;
			MinD.z = Dotz;
		}
		if (Dotz > MaxD.z)
		{
			Maxz = Vertex.z;
			MaxD.z = Dotz;
		}
	}

	RBoundingBox Bb{};
	Bb.Set(vec3(Minx, Miny, Minz), vec3(Maxx, Maxy, Maxz));
	return Bb;
}

void RMesh::ComputeTangentsAndBitangents()
{
	// @TODO: This may lead to bugs, we currentyl assume here that faces = 2 triangles each, and while that may hold true with the current loader, that may not remain the case forever.
	For(FacesCount)
	{
		RVertex V1 = this->Vertices[Indices[i * 3 + 0]];
		RVertex V2 = this->Vertices[Indices[i * 3 + 1]];
		RVertex V3 = this->Vertices[Indices[i * 3 + 2]];

		vec3 Edge1 = V2.Position - V1.Position;
		vec3 Edge2 = V3.Position - V1.Position;
		vec2 DeltaUv1 = V2.TexCoords - V1.TexCoords;
		vec2 DeltaUv2 = V3.TexCoords - V1.TexCoords;

		float F = 1.0f / (DeltaUv1.x * DeltaUv2.y - DeltaUv2.x * DeltaUv1.y);

		vec3 Tangent, Bitangent;
		Tangent.x = F * (DeltaUv2.y * Edge1.x - DeltaUv1.y * Edge2.x);
		Tangent.y = F * (DeltaUv2.y * Edge1.y - DeltaUv1.y * Edge2.y);
		Tangent.z = F * (DeltaUv2.y * Edge1.z - DeltaUv1.y * Edge2.z);

		Bitangent.x = F * (-DeltaUv2.x * Edge1.x + DeltaUv1.x * Edge2.x);
		Bitangent.y = F * (-DeltaUv2.x * Edge1.y + DeltaUv1.x * Edge2.y);
		Bitangent.z = F * (-DeltaUv2.x * Edge1.z + DeltaUv1.x * Edge2.z);

		this->Vertices[Indices[i * 3 + 0]].Tangent = Tangent;
		this->Vertices[Indices[i * 3 + 0]].Bitangent = Bitangent;

		this->Vertices[Indices[i * 3 + 1]].Tangent = Tangent;
		this->Vertices[Indices[i * 3 + 1]].Bitangent = Bitangent;

		this->Vertices[Indices[i * 3 + 2]].Tangent = Tangent;
		this->Vertices[Indices[i * 3 + 2]].Bitangent = Bitangent;
	}
}

RGLData setup_gl_data_for_lines(const RVertex* Vertices, uint Size)
{
	RGLData GlData;

	// create buffers/arrays
	glGenVertexArrays(1, &GlData.VAO);
	glGenBuffers(1, &GlData.VBO);

	// load data into vertex buffers
	glBindVertexArray(GlData.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, GlData.VBO);
	glBufferData(GL_ARRAY_BUFFER, Size * sizeof(RVertex), Vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RVertex), static_cast<void*>(nullptr));

	return GlData;
}

vector<RVertex> ConstructCylinder(float Radius, float HalfLenght, int Slices)
{
	vector<RVertex> Vertices;
	for (int i = 0; i < Slices; i++)
	{
		float Theta = static_cast<float>(i) * 2.0 * PI * (1.0 / Slices);
		float NextTheta = (static_cast<float>(i) + 1) * 2.0 * PI * (1.0 / Slices);
		// vertex at middle of end  
		auto Vertex = RVertex{vec3(0.0), vec3(HalfLenght), vec3(0.0)};
		Vertices.push_back(Vertex);
		//vertices at edges of circle 
		Vertex = RVertex{vec3(Radius * cos(Theta), HalfLenght, Radius * sin(Theta))};
		Vertices.push_back(Vertex);
		Vertex = RVertex{vec3(Radius * cos(NextTheta), HalfLenght, Radius * sin(NextTheta))};
		Vertices.push_back(Vertex);
		// the same vertices at the bottom of the cylinder (half face)
		Vertex = RVertex{vec3(Radius * cos(NextTheta), -HalfLenght, Radius * sin(NextTheta))};
		Vertices.push_back(Vertex);
		Vertex = RVertex{vec3(Radius * cos(Theta), -HalfLenght, Radius * sin(Theta))};
		Vertices.push_back(Vertex);
		// other half face
		Vertex = RVertex{vec3(Radius * cos(Theta), HalfLenght, Radius * sin(Theta))};
		Vertices.push_back(Vertex);
		Vertex = RVertex{vec3(Radius * cos(NextTheta), HalfLenght, Radius * sin(NextTheta))};
		Vertices.push_back(Vertex);
		// back from the middle
		Vertex = RVertex{vec3(Radius * cos(NextTheta), -HalfLenght, Radius * sin(NextTheta))};
		Vertices.push_back(Vertex);
		Vertex = RVertex{vec3(0.0, -HalfLenght, 0.0)};
		Vertices.push_back(Vertex);
		// roundabout
		Vertex = RVertex{vec3(Radius * cos(Theta), -HalfLenght, Radius * sin(Theta))};
		Vertices.push_back(Vertex);
		Vertex = RVertex{vec3(Radius * cos(NextTheta), -HalfLenght, Radius * sin(NextTheta))};
		Vertices.push_back(Vertex);
		Vertex = RVertex{vec3(0.0, -HalfLenght, 0.0)};
		Vertices.push_back(Vertex);
	}

	return Vertices;
}

// -----------------------------------------
// > GET TRIANGLE FOR COLLIDER INDEXED MESH
// -----------------------------------------
RTriangle get_triangle_for_collider_indexed_mesh(const RMesh* Mesh, int TriangleIndex)
{
	auto AIndex = Mesh->Indices[3 * TriangleIndex + 0];
	auto BIndex = Mesh->Indices[3 * TriangleIndex + 1];
	auto CIndex = Mesh->Indices[3 * TriangleIndex + 2];

	auto A = Mesh->Vertices[AIndex].Position;
	auto B = Mesh->Vertices[BIndex].Position;
	auto C = Mesh->Vertices[CIndex].Position;

	return RTriangle{A, B, C};
}

RTriangle get_triangle_for_collider_indexed_mesh(const RCollisionMesh* Mesh, int TriangleIndex)
{
	auto AIndex = Mesh->Indices[3 * TriangleIndex + 0];
	auto BIndex = Mesh->Indices[3 * TriangleIndex + 1];
	auto CIndex = Mesh->Indices[3 * TriangleIndex + 2];

	auto A = Mesh->Vertices[AIndex];
	auto B = Mesh->Vertices[BIndex];
	auto C = Mesh->Vertices[CIndex];

	return RTriangle{A, B, C};
}

// --------------------------------
// > GET TRIANGLE FOR INDEXED MESH
// --------------------------------
RTriangle GetTriangleForIndexedMesh(RMesh* Mesh, glm::mat4 MatModel, int TriangleIndex)
{
	auto AIndex = Mesh->Indices[3 * TriangleIndex + 0];
	auto BIndex = Mesh->Indices[3 * TriangleIndex + 1];
	auto CIndex = Mesh->Indices[3 * TriangleIndex + 2];

	auto AVertex = Mesh->Vertices[AIndex].Position;
	auto BVertex = Mesh->Vertices[BIndex].Position;
	auto CVertex = Mesh->Vertices[CIndex].Position;

	auto A = MatModel * glm::vec4(AVertex, 1.0);
	auto B = MatModel * glm::vec4(BVertex, 1.0);
	auto C = MatModel * glm::vec4(CVertex, 1.0);

	return RTriangle{A, B, C};
}
