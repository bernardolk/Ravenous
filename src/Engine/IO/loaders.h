#pragma once

#include "engine/core/core.h"

enum class RenderMethodEnum
{
	Triangles = 0x0004,
};

RMesh* LoadWavefrontObjAsMesh(const string& Filename);

RCollisionMesh* LoadWavefrontObjAsCollisionMesh(const string& Filename);
unsigned int LoadTextureFromFile(const string& Filename, const string& Directory);
void AttachExtraDataToMesh(string Filename, RMesh* Mesh);
void LoadMeshExtraData(string Filename, RMesh* Mesh);
void WriteMeshExtraDataFile(string Filename, RMesh* Mesh);
void LoadTexturesFromAssetsFolder();
vector<string> GetFilesInFolder(const string& Directory);
void LoadShaders();
void LoadModels();

void ExportWavefrontCollisionMesh(RCollisionMesh* CollisionMesh);
void ExportMeshBinary(RMesh* Mesh);
void ImportMeshBinary(const string& Filename);
bool DoesFileExist(const string& Filepath);