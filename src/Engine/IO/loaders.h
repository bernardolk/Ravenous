#pragma once

#include <engine/core/core.h>

enum class RenderMethodEnum
{
	triangles = 0x0004,
};

using StrVec = std::vector<std::string>;

Mesh* LoadWavefrontObjAsMesh(
	const std::string& path,
	const std::string& filename,
	const std::string& name = "",
	bool setup_gl_data = true,
	RenderMethodEnum render_method = RenderMethodEnum::triangles);

CollisionMesh* LoadWavefrontObjAsCollisionMesh(std::string path, std::string filename, std::string name = "");
unsigned int LoadTextureFromFile(const std::string& filename, const std::string& directory, bool gamma = false);
void AttachExtraDataToMesh(std::string filename, std::string filepath, Mesh* mesh);
void LoadMeshExtraData(std::string filename, Mesh* mesh);
void WriteMeshExtraDataFile(std::string filename, Mesh* mesh);
void LoadTexturesFromAssetsFolder();
StrVec GetFilesINFolder(std::string directory);
void LoadShaders();
