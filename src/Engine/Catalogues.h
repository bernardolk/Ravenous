#pragma once

#include "engine/core/core.h"
#include "engine/geometry/mesh.h"


struct REntityAttributes
{
	std::string name = "NONAME";
	std::string mesh = "aabb";
	std::string shader = "model";
	std::string texture = "grey";
	std::string collision_mesh = "aabb";
	//EntityType type = EntityType_Static;
	vec3 scale = vec3{1.0f};
};

struct RCatalogueSearchResult
{
	RTexture textures[2];
	int textures_found = 0;
	RMesh* mesh{};
	RCollisionMesh* collision_mesh{};
	RShader* shader{};
};


RCatalogueSearchResult FindEntityAssetsInCatalogue(const string& mesh, const string& collision_mesh, const string& shader, const string& texture);