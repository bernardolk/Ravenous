#pragma once

#include "engine/core/core.h"
#include "engine/entities/allocator/entity_pool.h"
#include "engine/entities/entity.h"
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

struct EntityManager
{
	// ------------------
	// > ENTITY MANAGER
	// ------------------ 
	u64 next_entity_id = 1;
	u64 editor_count = 0;
	EntityPool pool = EntityPool(200);

	std::vector<Entity*> deletion_stack;
	std::vector<Entity*>* entity_registry;
	std::vector<Entity*>* checkpoints_registry;
	std::vector<Entity*>* interactables_registry;

	World* world;

	// methods
	static EntityManager* Get()
	{
		static EntityManager instance;
		return &instance;
	}

	void SetEntityRegistry(std::vector<Entity*>* registry);
	void SetCheckpointsRegistry(std::vector<Entity*>* registry);
	void SetInteractablesRegistry(std::vector<Entity*>* registry);
	void SetWorld(World* world);
	void RegisterInWorldAndScene(Entity* entity) const;

	Entity* CreateEditorEntity(const EntityAttributes& attrs);
	Entity* CreateEntity(const EntityAttributes& attrs);
	Entity* CopyEntity(E_Entity* entity);

	[[nodiscard]] static CatalogueSearchResult FindEntityAssetsInCatalogue(const string& mesh, const string& collision_mesh, const string& shader, const string& texture);

	void RemoveFromCheckpointRegistry(Entity* entity) const;
	void RemoveInteractivity(Entity* entity);
	void MakeInteractable(Entity* entity);
	void UnsetAllTypeRelatedConfigurations(Entity* entity);
	// void SetType(Entity* entity, EntityType type);
	void MarkForDeletion(E_Entity* entity);
	void SafeDeleteMarkedEntities();
};
