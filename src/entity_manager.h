struct EntityManager
{
   // ---------------
   // ENTITY MANAGER
   // ---------------
   unsigned int count = 0;
   Shader* default_shader;
   Mesh* default_mesh;
   Texture default_texture;
   vector<Entity*> deletion_stack;
   vector<Entity*>* entity_registry;
   
   // -------------------------------------
   // [INTERNAL] FIND ASSETS IN CATALOGUES
   // -------------------------------------
   auto _find_entity_assets_in_catalogue(string mesh, string shader, string texture)
   {
      struct {
         Texture texture;
         Mesh* mesh;
         Shader* shader;
      } attrs;

      auto _texture = Texture_Catalogue.find(texture);
      if(_texture == Texture_Catalogue.end())
      {
         cout << "FATAL: texture'" << texture << "' not found in texture catalogue.\n";
         assert(false);
      }

      auto _shader = Shader_Catalogue.find(shader);
      if(_shader == Shader_Catalogue.end())
      {
         cout << "FATAL: shader'" << shader << "' not found in shader catalogue.\n";
         assert(false);
      }

      auto _mesh = Geometry_Catalogue.find(mesh);
      if(_mesh == Geometry_Catalogue.end())
      {
         cout << "FATAL: mesh'" << mesh << "' not found in mesh catalogue.\n";
         assert(false);
      }

      attrs.texture = _texture->second;
      attrs.mesh    = _mesh->second;
      attrs.shader  = _shader->second;
      return attrs;
   }

   // --------------------------
   // DEFAULT ENTITY ATTRIBUTES
   // --------------------------
   void set_default_entity_attributes(string mesh, string shader, string texture)
   {
      auto [_texture, _mesh, _shader] = _find_entity_assets_in_catalogue(mesh, shader, texture);
      default_texture = _texture;
      default_shader  = _shader;
      default_mesh    = _mesh;
   }

   void set_entity_registry(vector<Entity*>* registry)
   {
      entity_registry = registry;
   }

   // ----------------
   // REGISTER ENTITY
   // ----------------
   void register_entity(Entity* entity)
   {
      G_SCENE_INFO.active_scene->entities.push_back(entity);
      World.update_entity_world_cells(entity);
   }

   // ---------------
   // CREATE ENTITY
   // ---------------
   Entity* create_entity(string name, string mesh, string shader, string texture)
   {
      auto [_texture, _mesh, _shader] = _find_entity_assets_in_catalogue(mesh, shader, texture);

      auto new_entity = new Entity();
      new_entity->id = ++count;
      new_entity->name = name;
      new_entity->textures.push_back(_texture);
      new_entity->shader = _shader;
      new_entity->mesh = _mesh;

      register_entity(new_entity);
      return new_entity;
   }

   Entity* create_entity(string name, bool load_defaults = true)
   {
      //warning: we don't check if name exists in registry
      auto new_entity = create_entity(load_defaults);
      new_entity->name = name;
      return new_entity;
   }

   Entity* create_entity(bool load_defaults = true)
   {
      auto new_entity = new Entity();
      new_entity->id = ++count;
      if(load_defaults)
      {
         new_entity->textures.push_back(default_texture);
         new_entity->shader = default_shader;
         new_entity->mesh = default_mesh;
      }
      register_entity(new_entity);
      return new_entity;  
   }

   // ------------
   // COPY ENTITY
   // ------------
   Entity* copy_entity(Entity* entity)
   {
      // allocate entity with new id
      auto new_entity = new Entity();
      new_entity->id = ++count;
      // copy static values
      *new_entity = *entity;
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
      register_entity(new_entity);
      return new_entity;
   }

   // --------------
   // DELETE ENTITY
   // --------------
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






