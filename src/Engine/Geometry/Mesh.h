#pragma once

#include "engine/core/core.h"
#include "vertex.h"

struct RGLData
{
	uint VAO = 0;
	uint VBO = 0;
	uint EBO = 0;
};

// Note: There's an @optimization opportunity by using glDrawElements instead of glDrawArray but that would require restructuring the RMesh data structure, most likely.

struct RMesh
{
	vector<RVertex> Vertices;
	vector<uint> Indices;
	uint FacesCount;
	uint RenderMethod = 0x0004;
	RGLData GLData;
	string Name;
	// FILETIME last_written;

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


RGLData SetupGlDataForLines(const RVertex* Vertices, uint Size);
vector<RVertex> ConstructCylinder(float Radius, float HalfLenght, int Slices);
RTriangle GetTriangleForColliderIndexedMesh(const RMesh* Mesh, int TriangleIndex);
RTriangle GetTriangleForColliderIndexedMesh(const RCollisionMesh* Mesh, int TriangleIndex);
RTriangle GetTriangleForIndexedMesh(RMesh* Mesh, glm::mat4 MatModel, int TriangleIndex);
