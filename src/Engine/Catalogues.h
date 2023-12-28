#pragma once

#include "engine/core/core.h"
#include "engine/geometry/mesh.h"


struct REntityAttributes
{
	string Name = "NONAME";
	string Mesh = "aabb";
	string Shader = "model";
	string Texture = "grey";
	string CollisionMesh = "aabb";
	vec3 Scale = vec3{1.0f};
};

struct RCatalogueSearchResult
{
	RTexture Textures[2];
	int TexturesFound = 0;
	RMesh* Mesh = nullptr;
	RCollisionMesh* CollisionMesh = nullptr;
	RShader* Shader = nullptr;
};


RCatalogueSearchResult FindEntityAssetsInCatalogue(const string& MeshName, const string& CollisionMeshName, const string& ShaderName, const string& TextureName);

RMesh* GetOrLoadMesh(const string& MeshName);
RCollisionMesh* GetOrLoadCollisionMesh(const string& CollisionMeshName);
RShader* GetShader(const string& ShaderName);
