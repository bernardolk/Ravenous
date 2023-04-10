#pragma once

#include "engine/core/core.h"
#include "engine/geometry/mesh.h"


struct EntityAttributes
{
	std::string name = "NONAME";
	std::string mesh = "aabb";
	std::string shader = "model";
	std::string texture = "grey";
	std::string collision_mesh = "aabb";
	//EntityType type = EntityType_Static;
	vec3 scale = vec3{1.0f};
};

struct CatalogueSearchResult
{
	Texture textures[2];
	int textures_found = 0;
	Mesh* mesh{};
	CollisionMesh* collision_mesh{};
	Shader* shader{};
};


CatalogueSearchResult FindEntityAssetsInCatalogue(const string& mesh, const string& collision_mesh, const string& shader, const string& texture);