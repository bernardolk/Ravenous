struct EntityAttributes {
   string name;
   string mesh;
   string shader;
   string texture;
   string collision_mesh;
   EntityType type;
   vec3 scale = vec3{1.0f};
};

struct EntityManager
{
   // ------------------
   // > ENTITY MANAGER
   // ------------------ 
   u64 next_entity_id = 1;
   u64 editor_count = 0;
   EntityPool pool = EntityPool(200);

   Shader*     default_shader;
   Mesh*       default_mesh;
   Texture     default_texture;

   vector<Entity*>   deletion_stack;
   vector<Entity*>*  entity_registry;
   vector<Entity*>*  checkpoints_registry;
   vector<Entity*>*  interactables_registry;
   
   // --------------------------------------
   // > FIND ASSETS IN CATALOGUES
   // --------------------------------------
   // [INTERNAL]
   auto _find_entity_assets_in_catalogue(string mesh, string collision_mesh, string shader, string texture)
   {
      struct {
         Texture textures[2];
         int textures_found = 0;
         Mesh* mesh;
         Mesh* collision_mesh;
         Shader* shader;
      } attrs;

      if(mesh != "")
      {
         auto _mesh = Geometry_Catalogue.find(mesh);
         if(_mesh == Geometry_Catalogue.end())
         {
            attrs.mesh = load_wavefront_obj_as_mesh(MODELS_PATH, mesh);
         }
         else
            attrs.mesh = _mesh->second;
      }
      
      if(collision_mesh != "")
      {
         auto _collision_mesh = Geometry_Catalogue.find(collision_mesh);
         if(_collision_mesh == Geometry_Catalogue.end())
         {
            attrs.collision_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, collision_mesh);
         }
         else
            attrs.collision_mesh = _collision_mesh->second;
      }

      if(shader != "")
      {
         auto _shader = Shader_Catalogue.find(shader);
         if(_shader == Shader_Catalogue.end())
         {
            std::cout << "FATAL: shader'" << shader << "' not found in shader catalogue.\n";
            assert(false);
         }
         attrs.shader = _shader->second;
      }

      if(texture != "")
      {
         // diffuse texture
         {
            auto _texture = Texture_Catalogue.find(texture);
            if(_texture == Texture_Catalogue.end())
            {
               std::cout << "FATAL: texture'" << texture << "' not found in texture catalogue.\n";
               assert(false);
            }
            attrs.textures[0] = _texture->second;
            attrs.textures_found++;
         }

         // normal texture
         {
            auto _texture = Texture_Catalogue.find(texture + "_normal");
            if(_texture != Texture_Catalogue.end())
            {
               attrs.textures[1] = _texture->second;
               attrs.textures_found++;
            }
         }
      }

      return attrs;
   }

   // ---------------------------------
   // > SET DEFAULT ENTITY ATTRIBUTES
   // ---------------------------------
   void set_default_entity_attributes(string mesh, string shader, string texture)
   {
      auto [_textures, _texture_count, _mesh, _,  _shader] = _find_entity_assets_in_catalogue(mesh, "", shader, texture);
      default_texture = _textures[0];
      default_shader  = _shader;
      default_mesh    = _mesh;
   }

   // ---------------------------------
   // > SET REGISTRIES
   // ---------------------------------
   void set_entity_registry(vector<Entity*>* registry)
   {
      entity_registry = registry;
   }

   void set_checkpoints_registry(vector<Entity*>* registry)
   {
      checkpoints_registry = registry;
   }

   void set_interactables_registry(vector<Entity*>* registry)
   {
      interactables_registry = registry;
   }

   // ------------------
   // > REGISTER ENTITY
   // ------------------
   void register_in_world_and_scene(Entity* entity)
   {
      G_SCENE_INFO.active_scene->entities.push_back(entity);
      entity->update();
      World.update_entity_world_cells(entity);
      World.update_cells_in_use_list();
   }

   // -----------------
   // > CREATE ENTITY
   // -----------------
   // Deals with entity creation. All entities created should be created through here.

   // MAIN FUNCTION
   Entity* create_entity(
      string name,
      string mesh,
      string shader,
      string texture,
      string collision_mesh,
      vec3 scale = vec3{1.0f})
   {
      auto [_textures, _texture_count,  _mesh, _collision_mesh, _shader] = 
         _find_entity_assets_in_catalogue(mesh, collision_mesh, shader, texture);

      Entity* new_entity                              = pool.get_next();
      new_entity->name                                = name;
      new_entity->shader                              = _shader;
      new_entity->mesh                                = _mesh;
      new_entity->scale                               = scale;
      new_entity->collision_mesh                      = _collision_mesh;
      new_entity->collider                            = *_collision_mesh;
      new_entity->collider.name                       = name + "-collider";
      new_entity->collider.setup_gl_data();
      For(_texture_count)
         new_entity->textures.push_back(_textures[i]);

      register_in_world_and_scene(new_entity);
      return new_entity;
   }

   // ----------------------------------------
   // > > Creates based on attributes struct
   // ----------------------------------------
   Entity* create_entity(EntityAttributes* attrs)
   {
      auto new_entity = create_entity(
         attrs->name,
         attrs->mesh,
         attrs->shader,
         attrs->texture,
         attrs->collision_mesh,
         attrs->scale
      );

      // sets new entity_type
      set_type(new_entity, attrs->type);

      return new_entity;
   }

   // -----------------------------------------------------------
   // > > Creates with a said name and maybe defaults or blank
   // -----------------------------------------------------------
   Entity* create_entity(string name, bool load_defaults = true)
   {
      //warning: we don't check if name exists in registry
      auto new_entity = create_entity(load_defaults);
      new_entity->name = name;
      return new_entity;
   }

   // ------------------------------------
   // > > Creates with defaults or blank
   // ------------------------------------
   Entity* create_entity(bool load_defaults = false)
   {
      auto new_entity = pool.get_next();
      if(load_defaults)
      {
         new_entity->textures.push_back(default_texture);
         new_entity->shader                  = default_shader;
         new_entity->mesh                    = default_mesh;
         new_entity->collision_mesh          = default_mesh;
         register_in_world_and_scene(new_entity);
      }
      return new_entity;  
   }

   // -----------------------
   // > CREATE EDITOR ENTITY
   // -----------------------
   // Editor entities can be created using this method. These entities have separate id's and are not
   //    registered into the world.

   Entity* create_editor_entity(
      string name,
      string mesh,
      string shader,
      string texture,
      string collision_mesh,
      vec3 scale = vec3{1.0f})
   {
      auto [_textures, _texture_count, _mesh, _collision_mesh, _shader] =
          _find_entity_assets_in_catalogue(mesh, collision_mesh, shader, texture);

      Entity* new_entity                              = pool.get_next();
      new_entity->id                                  = ++editor_count;
      new_entity->name                                = name;
      new_entity->shader                              = _shader;
      new_entity->mesh                                = _mesh;
      new_entity->scale                               = scale;
      new_entity->collision_mesh                      = _collision_mesh;
      new_entity->collider                            = *_collision_mesh;
      new_entity->collider.name                       = name + "-collider";
      new_entity->collider.setup_gl_data();
      For(_texture_count)
         new_entity->textures.push_back(_textures[i]);

      return new_entity;
   }


   // ---------------
   // > COPY ENTITY
   // ---------------
   Entity* copy_entity(Entity* entity)
   {
      // allocate entity with new id
      auto new_entity               = pool.get_next();
      *new_entity                   = *entity;
      new_entity->id                = next_entity_id++;
      new_entity->collider          = *new_entity->collision_mesh;
      new_entity->collider.setup_gl_data();
      // tries new name with copy
      string new_name = new_entity->name;
      if(new_name != "NONAME")
      {
         new_name = new_name + " copy";
         // if exists already, keep increasing the number inside parenthesis
         if(G_SCENE_INFO.active_scene->search_name(new_name))
         {
            unsigned int n_count = 1;
            do{
               new_name = new_name + "(" + to_string(n_count++) + ")";
            } while(G_SCENE_INFO.active_scene->search_name(new_name));
         }
      }
      new_entity->name = new_name;
      register_in_world_and_scene(new_entity);
      return new_entity;
   }

   // ------------------
   // > SET ENTITY TYPE
   // ------------------
   void _remove_from_checkpoint_registry(Entity* entity)
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

   void _remove_interactivity(Entity* entity)
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

   void _make_interactable(Entity* entity)
   {
      auto find = Geometry_Catalogue.find("trigger");
      if(find ==  Geometry_Catalogue.end())
         Quit_fatal("Couldn't find 'trigger' mesh for creating Trigger type entity.");
      
      entity->trigger = find->second;
      interactables_registry->push_back(entity);
   }

   void _unset_all_type_related_configurations(Entity* entity)
   {
      _remove_from_checkpoint_registry(entity);
      _remove_interactivity(entity);

      auto default_shader = Shader_Catalogue.find(DEFAULT_ENTITY_SHADER)->second;
      entity->shader = default_shader;
   }

   void set_type(Entity* entity, EntityType type)
   {  
      _unset_all_type_related_configurations(entity);

      switch(type)
      {
         // CHECKPOINT
         case EntityType_Checkpoint:
         {
            _make_interactable(entity);
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
            _make_interactable(entity);
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

            auto shader = Shader_Catalogue.find(ENTITY_SHADER_MARKING)->second;
            entity->shader = shader;
            break;
         }

         default:
            Quit_fatal("Entity manager doesn't know what entity type '" + to_string(type) + "' should be.");
      }
   }




   // ----------------
   // > DELETE ENTITY
   // ----------------
   void mark_for_deletion(Entity* entity)
   {
      // remove from scene render list
      int index = -1;
      For(entity_registry->size())
      {
         auto item = (*entity_registry)[i];
         if(item->id == entity->id)             //@todo: maybe we could check here by ptr address directly
         {
            index = i;
            break;
         }
      }
      if(index > -1) (*entity_registry).erase((*entity_registry).begin() + index);

      // remove from world cells
      for(int i = 0; i < entity->world_cells_count; i++)
         entity->world_cells[i]->remove(entity);

      // remove from checkpoint registry if checkpoint
      if(entity->type == EntityType_Checkpoint)
      {
         auto& vec = *(checkpoints_registry);
         vec.erase(std::remove(vec.begin(), vec.end(), entity), vec.end());
      }

      deletion_stack.push_back(entity);
   }

   void safe_delete_marked_entities()
   {
      // WARNING: ONLY EXECUTE AT THE END OF THE FRAME
      while(deletion_stack.size() > 0)
      {
         auto entity = deletion_stack[0];
         pool.free_slot(entity);
         deletion_stack.erase(deletion_stack.begin());
      }
   }
};







