#include <engine/core/rvn_types.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm> //utils
#include <engine/vertex.h> //utils
#include <utils.h>
#include <rvn_macros.h>
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
auto EntityManager::_find_entity_assets_in_catalogue(const std::string& mesh, const std::string& collision_mesh, const std::string& shader, const std::string& texture) const
{
   struct {
      Texture textures[2];
      int textures_found = 0;
      Mesh* mesh{};
      CollisionMesh* collision_mesh{};
      Shader* shader{};
   } attrs;

   Mesh* _mesh;
   if (mesh != "")
   {
      auto find_mesh = Geometry_Catalogue.find(mesh);
      if (find_mesh != Geometry_Catalogue.end())
         _mesh = find_mesh->second;
      else
         _mesh = load_wavefront_obj_as_mesh(MODELS_PATH, mesh);

      attrs.mesh = _mesh;
   }

   if (collision_mesh != "")
   {
      auto find_c_mesh = Collision_Geometry_Catalogue.find(collision_mesh);
      if (find_c_mesh != Collision_Geometry_Catalogue.end())
         attrs.collision_mesh = find_c_mesh->second;
      else
         attrs.collision_mesh = load_wavefront_obj_as_collision_mesh(MODELS_PATH, collision_mesh);
   }

   if (shader != "")
   {
      auto _shader = Shader_Catalogue.find(shader);
      if (_shader == Shader_Catalogue.end())
      {
         std::cout << "FATAL: shader'" << shader << "' not found in shader catalogue.\n";
         assert(false);
      }
      attrs.shader = _shader->second;
   }

   if (texture != "")
   {
      // diffuse texture
      {
         auto _texture = Texture_Catalogue.find(texture);
         if (_texture == Texture_Catalogue.end())
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
         if (_texture != Texture_Catalogue.end())
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
void EntityManager::set_default_entity_attributes(std::string mesh,std::string shader,std::string texture)
{
   auto [_textures, _texture_count, _mesh, _c_mesh,  _shader] = _find_entity_assets_in_catalogue(mesh, mesh, shader, texture);
   default_texture = _textures[0];
   default_shader  = _shader;
   default_mesh    = _mesh;
   default_c_mesh = _c_mesh;
}

// ---------------------------------
// > SET REGISTRIES
// ---------------------------------
void EntityManager::set_entity_registry(std::vector<Entity*>* registry)
{
   entity_registry = registry;
}

void EntityManager::set_checkpoints_registry(std::vector<Entity*>* registry)
{
   checkpoints_registry = registry;
}

void EntityManager::set_interactables_registry(std::vector<Entity*>* registry)
{
   interactables_registry = registry;
}

void EntityManager::set_world(World* world)
{
   this->world = world;
}

// ------------------
// > REGISTER ENTITY
// ------------------
void EntityManager::register_in_world_and_scene(Entity* entity)
{
   this->world->entities.push_back(entity);
   entity->update();
   this->world->update_entity_world_cells(entity);
   this->world->update_cells_in_use_list();
}

// -----------------
// > CREATE ENTITY
// -----------------
// Deals with entity creation. All entities created should be created through here.

// MAIN FUNCTION
Entity* EntityManager::create_entity(
   const std::string& name,
   const std::string& mesh,
   const std::string& shader,
   const std::string& texture,
   const std::string& collision_mesh,
   const vec3 scale)
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
   For(_texture_count)
      new_entity->textures.push_back(_textures[i]);

   register_in_world_and_scene(new_entity);
   return new_entity;
}

// ----------------------------------------
// > > Creates based on attributes struct
// ----------------------------------------
Entity* EntityManager::create_entity(const EntityAttributes* attrs)
{
   const auto new_entity = create_entity(
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
Entity* EntityManager::create_entity(const std::string& name, const bool load_defaults)
{
   //warning: we don't check if name exists in registry
   const auto new_entity = create_entity(load_defaults);
   new_entity->name = name;
   return new_entity;
}

// ------------------------------------
// > > Creates with defaults or blank
// ------------------------------------
Entity* EntityManager::create_entity(const bool load_defaults)
{
   const auto new_entity = pool.get_next();
   if(load_defaults)
   {
      new_entity->textures.push_back(default_texture);
      new_entity->shader                  = default_shader;
      new_entity->mesh                    = default_mesh;
      new_entity->collision_mesh          = default_c_mesh;
      register_in_world_and_scene(new_entity);
   }
   return new_entity;  
}

// -----------------------
// > CREATE EDITOR ENTITY
// -----------------------
// Editor entities can be created using this method. These entities have separate id's and are not
//    registered into the world.

Entity* EntityManager::create_editor_entity(
   const std::string& name,
   const std::string& mesh,
   const std::string& shader,
   const std::string& texture,
   const std::string& collision_mesh,
   const vec3 scale)
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
   For(_texture_count)
      new_entity->textures.push_back(_textures[i]);

   return new_entity;
}


// ---------------
// > COPY ENTITY
// ---------------
Entity* EntityManager::copy_entity(Entity* entity)
{
   // allocate entity with new id
   auto new_entity               = pool.get_next();
   *new_entity                   = *entity;
   new_entity->id                = next_entity_id++;
   new_entity->collider          = *new_entity->collision_mesh;
   // tries new name with copy
   std::string new_name = new_entity->name;
   if(new_name != "NONAME")
   {
      new_name = new_name + " copy";
      // if exists already, keep increasing the number inside parenthesis
      if(G_SCENE_INFO.active_scene->search_name(new_name))
      {
         unsigned int n_count = 1;
         do{
            new_name = new_name + "(" + std::to_string(n_count++) + ")";
         } while(G_SCENE_INFO.active_scene->search_name(new_name));
      }
   }
   new_entity->name = new_name;
   register_in_world_and_scene(new_entity);
   return new_entity;
}

void EntityManager::set_type(Entity* entity, const EntityType type)
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

         const auto shader = Shader_Catalogue.find(ENTITY_SHADER_MARKING)->second;
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
void EntityManager::mark_for_deletion(Entity* entity)
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

void EntityManager::safe_delete_marked_entities()
{
   // WARNING: ONLY EXECUTE AT THE END OF THE FRAME
   while(deletion_stack.size() > 0)
   {
      auto entity = deletion_stack[0];
      pool.free_slot(entity);
      deletion_stack.erase(deletion_stack.begin());
   }
}

// --------------------------------------
// >> PRIVATE METHODS
// --------------------------------------

// ------------------
// > SET ENTITY TYPE
// ------------------
void EntityManager::_remove_from_checkpoint_registry(Entity* entity) const
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

void EntityManager::_remove_interactivity(Entity* entity)
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

void EntityManager::_make_interactable(Entity* entity)
{
   auto find = Geometry_Catalogue.find("trigger");
   if(find ==  Geometry_Catalogue.end())
      Quit_fatal("Couldn't find 'trigger' mesh for creating Trigger type entity.")
   
   entity->trigger = find->second;
   interactables_registry->push_back(entity);
}

void EntityManager::_unset_all_type_related_configurations(Entity* entity)
{
   _remove_from_checkpoint_registry(entity);
   _remove_interactivity(entity);

   entity->shader = Shader_Catalogue.find(DEFAULT_ENTITY_SHADER)->second;;
}