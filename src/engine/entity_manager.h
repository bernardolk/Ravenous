#pragma once

#include "engine/entity_pool.h"

struct World;
struct CollisionMesh;
struct EntityPool;
struct GlobalSceneInfo;
struct EntityPool;
struct Texture;

extern GlobalSceneInfo GSceneInfo;

struct EntityAttributes
{
	std::string name = "NONAME";
	std::string mesh = "aabb";
	std::string shader = "model";
	std::string texture = "grey";
	std::string collision_mesh = "aabb";
	EntityType type = EntityType_Static;
	vec3 scale = vec3{1.0f};
};

struct T_EntityManager
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
	void SetEntityRegistry(std::vector<Entity*>* registry);
	void SetCheckpointsRegistry(std::vector<Entity*>* registry);
	void SetInteractablesRegistry(std::vector<Entity*>* registry);
	void SetWorld(World* world);
	void RegisterInWorldAndScene(Entity* entity) const;

	Entity* CreateEditorEntity(const EntityAttributes& attrs);
	Entity* CreateEntity(const EntityAttributes& attrs);
	Entity* CopyEntity(Entity* entity);
	
	[[nodiscard]] static auto FindEntityAssetsInCatalogue(
	const std::string& mesh,
	const std::string& collision_mesh,
	const std::string& shader,
	const std::string& texture);

	void RemoveFromCheckpointRegistry(Entity* entity) const;
	void RemoveInteractivity(Entity* entity);
	void MakeInteractable(Entity* entity);
	void UnsetAllTypeRelatedConfigurations(Entity* entity);
	void SetType(Entity* entity, EntityType type);
	void MarkForDeletion(Entity* entity);
	void SafeDeleteMarkedEntities();
};
