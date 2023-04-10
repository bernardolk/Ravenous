#include "engine/catalogues.h"

#include "rvn.h"
#include "io/loaders.h"
#include "render/shader.h"

CatalogueSearchResult FindEntityAssetsInCatalogue(const string& mesh, const string& collision_mesh, const string& shader, const string& texture)
{
	CatalogueSearchResult attrs;

	if (!mesh.empty())
	{
		const auto find_mesh = GeometryCatalogue.find(mesh);
		if (find_mesh != GeometryCatalogue.end())
			attrs.mesh = find_mesh->second;
		else
			attrs.mesh = LoadWavefrontObjAsMesh(Paths::Models, mesh);
	}

	if (!collision_mesh.empty())
	{
		const auto find_c_mesh = CollisionGeometryCatalogue.find(collision_mesh);
		if (find_c_mesh != CollisionGeometryCatalogue.end())
			attrs.collision_mesh = find_c_mesh->second;
		else
			attrs.collision_mesh = LoadWavefrontObjAsCollisionMesh(Paths::Models, collision_mesh);
	}

	if (!shader.empty())
	{
		const auto _shader = ShaderCatalogue.find(shader);
		if (_shader == ShaderCatalogue.end())
		{
			std::cout << "FATAL: shader'" << shader << "' not found in shader catalogue.\n";
			assert(false);
		}
		attrs.shader = _shader->second;
	}

	if (!texture.empty())
	{
		// diffuse texture
		{
			const auto _texture = TextureCatalogue.find(texture);
			if (_texture == TextureCatalogue.end())
			{
				std::cout << "FATAL: texture'" << texture << "' not found in texture catalogue.\n";
				assert(false);
			}
			attrs.textures[0] = _texture->second;
			attrs.textures_found++;
		}

		// normal texture
		{
			const auto _texture = TextureCatalogue.find(texture + "_normal");
			if (_texture != TextureCatalogue.end())
			{
				attrs.textures[1] = _texture->second;
				attrs.textures_found++;
			}
		}
	}

	return attrs;
}
