#pragma once

#include <engine/core/core.h>

enum class RenderMethodEnum
{
	Triangles = 0x0004,
};

using StrVec = std::vector<string>;

RMesh* LoadWavefrontObjAsMesh(
	const string& Path,
	const string& Filename,
	const string& Name = "",
	bool SetupGlData = true,
	RenderMethodEnum RenderMethod = RenderMethodEnum::Triangles);

RCollisionMesh* LoadWavefrontObjAsCollisionMesh(const string& Path, const string& Filename);
unsigned int LoadTextureFromFile(const string& Filename, const string& Directory, bool Gamma = false);
void AttachExtraDataToMesh(string Filename, string Filepath, RMesh* Mesh);
void LoadMeshExtraData(string Filename, RMesh* Mesh);
void WriteMeshExtraDataFile(string Filename, RMesh* Mesh);
void LoadTexturesFromAssetsFolder();
StrVec GetFilesINFolder(string Directory);
void LoadShaders();

void ExportWavefrontCollisionMesh(RCollisionMesh* CollisionMesh);
