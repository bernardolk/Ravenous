struct EntityAttributes {
   string name;
   string mesh;
   string shader;
   string texture;
   string collision_mesh;
   CollisionGeometryEnum collision;
   EntityType type;
   vec3 scale = vec3{1.0f};
};

struct EntityManager
{
   // ------------------
   // > ENTITY MANAGER
   // ------------------
   unsigned int count = 0;
   Shader* default_shader;
   Mesh* default_mesh;
   Texture default_texture;
   vector<Entity*> deletion_stack;
   vector<Entity*>* entity_registry;
   vector<Entity*>* checkpoints_registry;
   
   // --------------------------------------
   // > FIND ASSETS IN CATALOGUES
   // --------------------------------------
   // [INTERNAL]
   auto _find_entity_assets_in_catalogue(string mesh, string collision_mesh, string shader, string texture)
   {
      struct {
         Texture texture;
         Mesh* mesh;
         Mesh* collision_mesh;
         Shader* shader;
      } attrs;

      if(mesh != "")
      {
         auto _mesh = Geometry_Catalogue.find(mesh);
         if(_mesh == Geometry_Catalogue.end())
         {
            cout << "FATAL: mesh'" << mesh << "' not found in mesh catalogue.\n";
            assert(false);
         }
         attrs.mesh = _mesh->second;
      }
      
      if(collision_mesh != "")
      {
         auto _collision_mesh = Geometry_Catalogue.find(collision_mesh);
         if(_collision_mesh == Geometry_Catalogue.end())
         {
            cout << "FATAL: mesh'" << collision_mesh << "' not found in mesh catalogue.\n";
            assert(false);
         }
         attrs.collision_mesh = _collision_mesh->second;
      }

      if(shader != "")
      {
         auto _shader = Shader_Catalogue.find(shader);
         if(_shader == Shader_Catalogue.end())
         {
            cout << "FATAL: shader'" << shader << "' not found in shader catalogue.\n";
            assert(false);
         }
         attrs.shader = _shader->second;
      }

      if(texture != "")
      {
         auto _texture = Texture_Catalogue.find(texture);
         if(_texture == Texture_Catalogue.end())
         {
            cout << "FATAL: texture'" << texture << "' not found in texture catalogue.\n";
            assert(false);
         }
         attrs.texture = _texture->second;
      }

      return attrs;
   }

   // ---------------------------------
   // > SET DEFAULT ENTITY ATTRIBUTES
   // ---------------------------------
   void set_default_entity_attributes(string mesh, string shader, string texture)
   {
      auto [_texture, _mesh, _,  _shader] = _find_entity_assets_in_catalogue(mesh, "", shader, texture);
      default_texture = _texture;
      default_shader  = _shader;
      default_mesh    = _mesh;
   }

   void set_entity_registry(vector<Entity*>* registry)
   {
      entity_registry = registry;
   }

   void set_checkpoints_registry(vector<Entity*>* registry)
   {
      checkpoints_registry = registry;
   }

   // ------------------
   // > REGISTER ENTITY
   // ------------------
   void register_in_world_and_scene(Entity* entity)
   {
      G_SCENE_INFO.active_scene->entities.push_back(entity);
      World.update_entity_world_cells(entity);
   }

   // -----------------
   // > CREATE ENTITY
   // -----------------
   // Deals with entity creation. All entities created should be created through here.

   Entity* create_entity(
      string name,
      string mesh,
      string shader,
      string texture,
      string collision_mesh,
      CollisionGeometryEnum collision = COLLISION_ALIGNED_BOX,
      vec3 scale = vec3{1.0f})
   {
      auto [_texture, _mesh, _collision_mesh, _shader] = _find_entity_assets_in_catalogue(mesh, collision_mesh, shader, texture);

      auto new_entity                                 = new Entity();
      new_entity->id                                  = ++count;
      new_entity->name                                = name;
      new_entity->shader                              = _shader;
      new_entity->mesh                                = _mesh;
      new_entity->scale                               = scale;
      new_entity->collision_mesh                      = _collision_mesh;
      new_entity->collider                            = *_collision_mesh;
      new_entity->collision_geometry_type             = collision;
      new_entity->textures.push_back(_texture);
      new_entity->old_update_collision_geometry();

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
         attrs->collision,
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
      auto new_entity = new Entity();
      new_entity->id = ++count;
      if(load_defaults)
      {
         new_entity->textures.push_back(default_texture);
         new_entity->shader                  = default_shader;
         new_entity->mesh                    = default_mesh;
         new_entity->collision_mesh          = default_mesh;
         new_entity->collision_geometry_type = COLLISION_ALIGNED_BOX;
      }
      register_in_world_and_scene(new_entity);
      new_entity->old_update_collision_geometry();
      return new_entity;  
   }


   // ---------------
   // > COPY ENTITY
   // ---------------
   Entity* copy_entity(Entity* entity)
   {
      // allocate entity with new id
      auto new_entity = new Entity();
      *new_entity     = *entity;
      new_entity->id  = ++count;
      // tries new name with copy
      string new_name = new_entity->name;
      if(new_name != "NONAME")
      {
         new_name =+ " copy";
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
   // [internal]
   void _remove_checkpoint_type(Entity* entity)
   {
      if(entity->type == CHECKPOINT)
      {
         int entity_index_in_vector = -1;
         int i = 0;
         for(auto it = checkpoints_registry->begin();
            it < checkpoints_registry->end(); 
            it++, i++)
         {
            if(*it == entity)
            {
               entity_index_in_vector = i;
               break;
            }
         }
         if(entity_index_in_vector > -1)
            checkpoints_registry->erase(checkpoints_registry->begin() + entity_index_in_vector);
      }
   }

   void _add_checkpoint_type(Entity* entity)
   {
      entity->type = CHECKPOINT;
      entity->trigger = Geometry_Catalogue.find("trigger")->second;
      checkpoints_registry->push_back(entity);
   }

   void set_type(Entity* entity, EntityType type)
   {  
      switch(type)
      {
         case CHECKPOINT:
         {
            if(entity->type != CHECKPOINT)
               _add_checkpoint_type(entity);
            break;
         }
         case STATIC:
         {
            if(entity->type == CHECKPOINT)
               _remove_checkpoint_type(entity);
            entity->type = STATIC;
            break;
         }
         default:
         {
            cout << "Entity manager doesn't know what entity type '" << to_string(type) << "' should be";
            assert(false);
         }
      }
   }

   // ----------------
   // > DELETE ENTITY
   // ----------------
   void mark_for_deletion(Entity* entity)
   {
      // remove from scene render list
      int index = -1;
      for (int i = 0; i < entity_registry->size(); i++)
      {
         auto item = (*entity_registry)[i];
         if(item->id == entity->id)
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
      if(entity->type == CHECKPOINT)
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
         deletion_stack.erase(deletion_stack.begin());
         delete entity;
      }
   }
};







