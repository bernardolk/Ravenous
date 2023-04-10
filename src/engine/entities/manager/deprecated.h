/* EntityManager.h */


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


/* ENeityManager.cpp */

	// ---------------------------------
	// > SET REGISTRIES
	// ---------------------------------

	// TODO: Obsolete with new entity system
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
	// TODO: Rethink for new entity system
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

	/*
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

		// RegisterInWorldAndScene(new_entity);

		// TODO: Obsolete with new entity system
		// sets new entity_type
		//SetType(new_entity, attrs.type);

		return new_entity;
	}
	*/

	// -----------------------
	// > CREATE EDITOR ENTITY
	// -----------------------
	// TODO: Better describe what Editor entity is. We should create a EditorWidgetManager for those, these are not real "entities".
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
	// TODO: Rethink with new entity system
	Entity* EntityManager::CopyEntity(E_Entity* entity)
	{
		DEPRECATED_BEGIN
		/*
		// allocate entity with new id
		auto new_entity = pool.GetNext();
		*new_entity = *entity;
		new_entity->id = next_entity_id++;
		new_entity->collider = *new_entity->collision_mesh;
		// tries new name with copy
		std::string new_name = new_entity->name;
		if (new_name != "NONAME")
		{
			auto* GSI = GlobalSceneInfo::Get();

			new_name = new_name + " copy";
			// if exists already, keep increasing the number inside parenthesis
			if (GSI->active_scene->SearchName(new_name))
			{
				unsigned int n_count = 1;
				do
				{
					new_name = new_name + "(" + std::to_string(n_count++) + ")";
				} while (GSI->active_scene->SearchName(new_name));
			}
		}
		new_entity->name = new_name;
		// RegisterInWorldAndScene(new_entity);
		return new_entity;
		DEPRECATED_END
		*/
		
		return nullptr;
	}

	// void EntityManager::SetType(Entity* entity, const EntityType type)
	// {
	// 	UnsetAllTypeRelatedConfigurations(entity);
	//
	// 	switch (type)
	// 	{
	// 		// CHECKPOINT
	// 		case EntityType_Checkpoint:
	// 		{
	// 			MakeInteractable(entity);
	// 			checkpoints_registry->push_back(entity);
	// 			entity->type = EntityType_Checkpoint;
	// 			break;
	// 		}
	//
	// 		// STATIC
	// 		case EntityType_Static:
	// 		{
	// 			entity->type = EntityType_Static;
	// 			break;
	// 		}
	//
	// 		// TIMER TRIGGER
	// 		case EntityType_TimerTrigger:
	// 		{
	// 			MakeInteractable(entity);
	// 			entity->type = EntityType_TimerTrigger;
	// 			// initialize union member
	// 			new(&entity->timer_trigger_data) TimerTriggerData();
	// 			break;
	// 		}
	//
	// 		// TIMER TARGET
	// 		case EntityType_TimerTarget:
	// 		{
	// 			entity->type = EntityType_TimerTarget;
	// 			new(&entity->timer_trigger_data) TimerTargetData();
	// 			break;
	// 		}
	//
	// 		// TIMER MARKING
	// 		case EntityType_TimerMarking:
	// 		{
	// 			entity->type = EntityType_TimerMarking;
	// 			new(&entity->timer_marking_data) TimerMarkingData();
	//
	// 			const auto shader = ShaderCatalogue.find(EntityShaderMarking)->second;
	// 			entity->shader = shader;
	// 			break;
	// 		}
	//
	// 		default:
	// 			Quit_fatal("Entity manager doesn't know what entity type '" + std::to_string(type) + "' should be.")
	// 	}
	// }

	// ----------------
	// > DELETE ENTITY
	// ----------------
	// TODO: Rethink with new entity system
	void EntityManager::MarkForDeletion(E_Entity* entity)
	{
		DEPRECATED_BEGIN
		/*
		// remove from scene render list
		int index = -1;
		For(entity_registry->size())
		{
			auto item = (*entity_registry)[i];
			if (item->id == entity->id) //@todo: maybe we could check here by ptr address directly
			{
				index = i;
				break;
			}
		}
		if (index > -1)
			(*entity_registry).erase((*entity_registry).begin() + index);

		// remove from world cells
		for (int i = 0; i < entity->world_cells_count; i++)
			entity->world_cells[i]->Remove(entity);

		// remove from checkpoint registry if checkpoint
		
		//if (entity->type == EntityType_Checkpoint)
		//{
		//	auto& vec = *(checkpoints_registry);
		//	std::erase(vec, entity);
		//}
		//
		deletion_stack.push_back(entity);
		*/
		DEPRECATED_END
		
		entity->deleted = true;
	}

	// TODO: Rethink with new entity system
	void EntityManager::SafeDeleteMarkedEntities()
	{
		// WARNING: ONLY EXECUTE AT THE END OF THE FRAME
		while (deletion_stack.size() > 0)
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
	// TODO: Obsolete with new entity system
	void EntityManager::RemoveFromCheckpointRegistry(Entity* entity) const
	{
		int index = -1;
		For(checkpoints_registry->size())
		{
			auto it = (*checkpoints_registry)[i];
			if (it == entity)
			{
				index = i;
				break;
			}
		}
		if (index > -1)
			checkpoints_registry->erase(checkpoints_registry->begin() + index);
	}

	// TODO: Obsolete with new entity system
	void EntityManager::RemoveInteractivity(Entity* entity)
	{
		int index = -1;
		For(interactables_registry->size())
		{
			auto it = (*interactables_registry)[i];
			if (it == entity)
			{
				index = i;
				break;
			}
		}

		if (index > -1)
			interactables_registry->erase(interactables_registry->begin() + index);

		entity->trigger = nullptr;
	}

	// TODO: Obsolete with new entity system
	void EntityManager::MakeInteractable(Entity* entity)
	{
		auto find = GeometryCatalogue.find("trigger");
		if (find == GeometryCatalogue.end())
			Quit_fatal("Couldn't find 'trigger' mesh for creating Trigger type entity.")

		entity->trigger = find->second;
		interactables_registry->push_back(entity);
	}

	// TODO: Obsolete with new entity system
	void EntityManager::UnsetAllTypeRelatedConfigurations(Entity* entity)
	{
		RemoveFromCheckpointRegistry(entity);
		RemoveInteractivity(entity);

		entity->shader = ShaderCatalogue.find(DefaultEntityShader)->second;
	}
