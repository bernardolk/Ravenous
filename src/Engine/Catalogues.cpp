#include "engine/catalogues.h"

#include "rvn.h"
#include "entities/Entity.h"
#include "io/loaders.h"
#include "render/Shader.h"

RCatalogueSearchResult FindEntityAssetsInCatalogue(const string& MeshName, const string& CollisionMeshName, const string& ShaderName, const string& TextureName)
{
	RCatalogueSearchResult Attrs;

	// TODO: Refactor Catalogues into objects that can themselves execute the finding/lazy loading code, so that these code blocks below are encapsulated and
	//	written just once for all asset catalogues. Later on, each catalogue can have its own allocation / loading strategy etc.
	if (!MeshName.empty())
	{
		Attrs.Mesh = GetOrLoadMesh(MeshName);
	}

	if (!CollisionMeshName.empty())
	{
		Attrs.CollisionMesh = GetOrLoadCollisionMesh(MeshName);
	}

	if (!ShaderName.empty())
	{
		Attrs.Shader = GetShader(ShaderName);
	}

	if (!TextureName.empty())
	{
		// diffuse texture
		{
			const auto Texture = TextureCatalogue.find(TextureName);
			if (Texture == TextureCatalogue.end())
				FatalError("FATAL: texture '%s' not found in texture catalogue.", TextureName.c_str());

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

RMesh* GetOrLoadMesh(const string& MeshName)
{
	auto** FindMesh = Find(GeometryCatalogue, MeshName);
	if (!FindMesh)
		return LoadWavefrontObjAsMesh(Paths::Models, MeshName);

	return *FindMesh;
}

RCollisionMesh* GetOrLoadCollisionMesh(const string& CollisionMeshName)
{
	auto** FindMesh = Find(CollisionGeometryCatalogue, CollisionMeshName);
	if (!FindMesh)
		return LoadWavefrontObjAsCollisionMesh(Paths::Models, CollisionMeshName);

	return *FindMesh;
}

RShader* GetShader(const string& ShaderName)
{
	auto** FindShader = Find(ShaderCatalogue, ShaderName);
	if (!FindShader)
	{
		Log("Shader '%s' not found.", ShaderName.c_str());
		auto** FindDefaultShader = Find(ShaderCatalogue, DefaultEntityShader);
		if (!FindDefaultShader)
		{
			FatalError("Default Entity Shader '%s' not found.", DefaultEntityShader.c_str());
		}

		return *FindDefaultShader;
	}

	return *FindShader;
}

RTexture GetOrLoadTexture(const string& TextureName)
{
	auto* FindTexture = Find(TextureCatalogue, TextureName);
	if (!FindTexture)
	{
		Log("Texture '%s' not found.", TextureName.c_str());
		return RTexture{};
	}

	return *FindTexture;
}
