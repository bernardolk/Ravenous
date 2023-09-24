#pragma once

#include <engine/core/core.h>

enum class RenderMethodEnum
{
	triangles = 0x0004,
};

using StrVec = std::vector<std::string>;

RMesh* LoadWavefrontObjAsMesh(
	const std::string& path,
	const std::string& filename,
	const std::string& name = "",
	bool setup_gl_data = true,
	RenderMethodEnum render_method = RenderMethodEnum::triangles);

RCollisionMesh* LoadWavefrontObjAsCollisionMesh(std::string path, std::string filename, std::string name = "");
unsigned int LoadTextureFromFile(const std::string& filename, const std::string& directory, bool gamma = false);
void AttachExtraDataToMesh(std::string filename, std::string filepath, RMesh* mesh);
void LoadMeshExtraData(std::string filename, RMesh* mesh);
void WriteMeshExtraDataFile(std::string filename, RMesh* mesh);
void LoadTexturesFromAssetsFolder();
StrVec GetFilesINFolder(std::string directory);
void LoadShaders();
