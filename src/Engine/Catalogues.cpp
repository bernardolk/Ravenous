#include "engine/catalogues.h"

#include "rvn.h"
#include "io/loaders.h"
#include "render/Shader.h"

RCatalogueSearchResult FindEntityAssetsInCatalogue(const string& MeshName, const string& CollisionMeshName, const string& ShaderName, const string& TextureName)
{
	RCatalogueSearchResult Attrs;

	if (!MeshName.empty())
	{
		const auto FindMesh = GeometryCatalogue.find(MeshName);
		if (FindMesh != GeometryCatalogue.end())
			Attrs.Mesh = FindMesh->second;
		else
			Attrs.Mesh = LoadWavefrontObjAsMesh(Paths::Models, MeshName);
	}

	if (!CollisionMeshName.empty())
	{
		const auto FindCollisionMesh = CollisionGeometryCatalogue.find(CollisionMeshName);
		if (FindCollisionMesh != CollisionGeometryCatalogue.end())
			Attrs.CollisionMesh = FindCollisionMesh->second;
		else
			Attrs.CollisionMesh = LoadWavefrontObjAsCollisionMesh(Paths::Models, CollisionMeshName);
	}

	if (!ShaderName.empty())
	{
		const auto Shader = ShaderCatalogue.find(ShaderName);
		if (Shader == ShaderCatalogue.end())
			fatal_error("FATAL: shader '%s' not found in shader catalogue.", ShaderName.c_str());

		Attrs.Shader = Shader->second;
	}

	if (!TextureName.empty())
	{
		// diffuse texture
		{
			const auto Texture = TextureCatalogue.find(TextureName);
			if (Texture == TextureCatalogue.end())
				fatal_error("FATAL: texture '%s' not found in texture catalogue.", TextureName.c_str());

			Attrs.Textures[0] = Texture->second;
			Attrs.TexturesFound++;
		}

		// normal texture
		{
			const auto Texture = TextureCatalogue.find(TextureName + "_normal");
			if (Texture != TextureCatalogue.end())
			{
				Attrs.Textures[1] = Texture->second;
				Attrs.TexturesFound++;
			}
		}
	}

	return Attrs;
}
