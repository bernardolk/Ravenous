#include <engine/core/types.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm> //utils
#include <engine/vertex.h> //utils
#include <utils.h>
#include <engine/logging.h>
#include <engine/rvn.h>
#include <engine/collision/primitives/bounding_box.h>
#include <glm/gtx/quaternion.hpp>
#include <engine/mesh.h>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/triangle.h>
#include <colors.h>
#include <engine/collision/collision_mesh.h>
#include <engine/entity.h>
#include <engine/lights.h>
#include <engine/shader.h>
#include <scene.h>
#include <engine/loaders.h>
#include <engine/world/world.h>
#include <engine/logging.h>
#include <engine/entity_pool.h>
#include <engine/entity_manager.h>

// --------------------------------------
// > FIND ASSETS IN CATALOGUES
// --------------------------------------

// @TODO: Refactor this into methods of catalogues
auto EntityManager::FindEntityAssetsInCatalogue(const std::string& mesh, const std::string& collision_mesh, const std::string& shader, const std::string& texture)
{
	struct
	{
		Texture textures[2];
		int textures_found = 0;
		Mesh* mesh{};
		CollisionMesh* collision_mesh{};
		Shader* shader{};
	} attrs;

	if(!mesh.empty())
	{
		const auto find_mesh = GeometryCatalogue.find(mesh);
		if(find_mesh != GeometryCatalogue.end())
			attrs.mesh = find_mesh->second;
		else
			attrs.mesh = load_wavefront_obj_as_mesh(Paths::Models, mesh);
	}

	if(!collision_mesh.empty())
	{
		const auto find_c_mesh = CollisionGeometryCatalogue.find(collision_mesh);
		if(find_c_mesh != CollisionGeometryCatalogue.end())
			attrs.collision_mesh = find_c_mesh->second;
		else
			attrs.collision_mesh = load_wavefront_obj_as_collision_mesh(Paths::Models, collision_mesh);
	}

	if(!shader.empty())
	{
		const auto _shader = ShaderCatalogue.find(shader);
		if(_shader == ShaderCatalogue.end())
		{
			std::cout << "FATAL: shader'" << shader << "' not found in shader catalogue.\n";
			assert(false);
		}
		attrs.shader = _shader->second;
	}

	if(!texture.empty())
	{
		// diffuse texture
		{
			const auto _texture = TextureCatalogue.find(texture);
			if(_texture == TextureCatalogue.end())
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
			if(_texture != TextureCatalogue.end())
			{
				attrs.textures[1] = _texture->second;
				attrs.textures_found++;
			}
		}
	}

	return attrs;
}

// ---------------------------------
// > SET REGISTRIES
// ---------------------------------
void EntityManager::SetEntityRegistry(std::vector<Entity*>* registry)
{
	entity_registry = registry;
}

void EntityManager::SetCheckpointsRegistry(std::vector<Entity*>* registry)
{
	checkpoints_registry = registry;
}

void EntityManager::SetInteractablesRegistry(std::vector<Entity*>* registry)
{
	interactables_registry = registry;
}

void EntityManager::SetWorld(World* world)
{
	this->world = world;
}

// ------------------
// > REGISTER ENTITY
// ------------------
void EntityManager::RegisterInWorldAndScene(Entity* entity) const
{
	entity->Update();
	this->world->entities.push_back(entity);
	this->world->UpdateEntityWorldCells(entity);
	this->world->UpdateCellsInUseList();
}

// -----------------
// > CREATE ENTITY
// -----------------
// Deals with entity creation. All entities created should be created through here.

Entity* EntityManager::CreateEntity(const EntityAttributes& attrs)
{
	auto [
		_textures,
		_texture_count,
		_mesh,
		_collision_mesh,
		_shader] = FindEntityAssetsInCatalogue(attrs.mesh, attrs.collision_mesh, attrs.shader, attrs.texture);

	Entity* new_entity = pool.GetNext();
	new_entity->name = attrs.name;
	new_entity->shader = _shader;
	new_entity->mesh = _mesh;
	new_entity->scale = attrs.scale;
	new_entity->collision_mesh = _collision_mesh;
	new_entity->collider = *_collision_mesh;

	For(_texture_count)
		new_entity->textures.push_back(_textures[i]);

	RegisterInWorldAndScene(new_entity);

	// sets new entity_type
	SetType(new_entity, attrs.type);

	return new_entity;
}

// -----------------------
// > CREATE EDITOR ENTITY
// -----------------------
// Editor entities can be created using this method. These entities have separate id's and are not
//    registered into the world.

Entity* EntityManager::CreateEditorEntity(const EntityAttributes& attrs)
{
	auto [_textures, _texture_count, _mesh, _collision_mesh, _shader] =
	FindEntityAssetsInCatalogue(attrs.mesh, attrs.collision_mesh, attrs.shader, attrs.texture);

	Entity* new_entity = pool.GetNext();
	new_entity->id = ++editor_count;
	new_entity->name = attrs.name;
	new_entity->shader = _shader;
	new_entity->mesh = _mesh;
	new_entity->scale = attrs.scale;
	new_entity->collision_mesh = _collision_mesh;
	new_entity->collider = *_collision_mesh;

	For(_texture_count)
		new_entity->textures.push_back(_textures[i]);

	return new_entity;
}


// ---------------
// > COPY ENTITY
// ---------------
Entity* EntityManager::CopyEntity(Entity* entity)
{
	// allocate entity with new id
	auto new_entity = pool.GetNext();
	*new_entity = *entity;
	new_entity->id = next_entity_id++;
	new_entity->collider = *new_entity->collision_mesh;
	// tries new name with copy
	std::string new_name = new_entity->name;
	if(new_name != "NONAME")
	{
		new_name = new_name + " copy";
		// if exists already, keep increasing the number inside parenthesis
		if(GSceneInfo.active_scene->SearchName(new_name))
		{
			unsigned int n_count = 1;
			do
			{
				new_name = new_name + "(" + std::to_string(n_count++) + ")";
			} while(GSceneInfo.active_scene->SearchName(new_name));
		}
	}
	new_entity->name = new_name;
	RegisterInWorldAndScene(new_entity);
	return new_entity;
}

void EntityManager::SetType(Entity* entity, const EntityType type)
{
	UnsetAllTypeRelatedConfigurations(entity);

	switch(type)
	{
		// CHECKPOINT
		case EntityType_Checkpoint:
		{
			MakeInteractable(entity);
			checkpoints_registry->push_back(entity);
			entity->type = EntityType_Checkpoint;
			break;
		}

		// STATIC
		case EntityType_Static:
		{
			entity->type = EntityType_Static;
			break;
		}

		// TIMER TRIGGER
		case EntityType_TimerTrigger:
		{
			MakeInteractable(entity);
			entity->type = EntityType_TimerTrigger;
			// initialize union member
			new(&entity->timer_trigger_data) TimerTriggerData();
			break;
		}

		// TIMER TARGET
		case EntityType_TimerTarget:
		{
			entity->type = EntityType_TimerTarget;
			new(&entity->timer_trigger_data) TimerTargetData();
			break;
		}

		// TIMER MARKING
		case EntityType_TimerMarking:
		{
			entity->type = EntityType_TimerMarking;
			new(&entity->timer_marking_data) TimerMarkingData();

			const auto shader = ShaderCatalogue.find(EntityShaderMarking)->second;
			entity->shader = shader;
			break;
		}

		default:
			Quit_fatal("Entity manager doesn't know what entity type '" + std::to_string(type) + "' should be.")
	}
}

// ----------------
// > DELETE ENTITY
// ----------------
void EntityManager::MarkForDeletion(Entity* entity)
{
	// remove from scene render list
	int index = -1;
	For(entity_registry->size())
	{
		auto item = (*entity_registry)[i];
		if(item->id == entity->id) //@todo: maybe we could check here by ptr address directly
		{
			index = i;
			break;
		}
	}
	if(index > -1)
		(*entity_registry).erase((*entity_registry).begin() + index);

	// remove from world cells
	for(int i = 0; i < entity->world_cells_count; i++)
		entity->world_cells[i]->Remove(entity);

	// remove from checkpoint registry if checkpoint
	if(entity->type == EntityType_Checkpoint)
	{
		auto& vec = *(checkpoints_registry);
		std::erase(vec, entity);
	}

	deletion_stack.push_back(entity);
}

void EntityManager::SafeDeleteMarkedEntities()
{
	// WARNING: ONLY EXECUTE AT THE END OF THE FRAME
	while(deletion_stack.size() > 0)
	{
		auto entity = deletion_stack[0];
		pool.FreeSlot(entity);
		deletion_stack.erase(deletion_stack.begin());
	}
}

// --------------------------------------
// >> PRIVATE METHODS
// --------------------------------------

// ------------------
// > SET ENTITY TYPE
// ------------------
void EntityManager::RemoveFromCheckpointRegistry(Entity* entity) const
{
	int index = -1;
	For(checkpoints_registry->size())
	{
		auto it = (*checkpoints_registry)[i];
		if(it == entity)
		{
			index = i;
			break;
		}
	}
	if(index > -1)
		checkpoints_registry->erase(checkpoints_registry->begin() + index);
}

void EntityManager::RemoveInteractivity(Entity* entity)
{
	int index = -1;
	For(interactables_registry->size())
	{
		auto it = (*interactables_registry)[i];
		if(it == entity)
		{
			index = i;
			break;
		}
	}

	if(index > -1)
		interactables_registry->erase(interactables_registry->begin() + index);

	entity->trigger = nullptr;
}

void EntityManager::MakeInteractable(Entity* entity)
{
	auto find = GeometryCatalogue.find("trigger");
	if(find == GeometryCatalogue.end())
		Quit_fatal("Couldn't find 'trigger' mesh for creating Trigger type entity.")

	entity->trigger = find->second;
	interactables_registry->push_back(entity);
}

void EntityManager::UnsetAllTypeRelatedConfigurations(Entity* entity)
{
	RemoveFromCheckpointRegistry(entity);
	RemoveInteractivity(entity);

	entity->shader = ShaderCatalogue.find(DefaultEntityShader)->second;
}
