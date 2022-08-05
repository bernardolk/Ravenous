#pragma once

struct World;
struct CollisionMesh;
struct EntityPool;
struct GlobalSceneInfo;
struct EntityPool;

extern GlobalSceneInfo G_SCENE_INFO;

struct EntityAttributes {
   std::string name;
   std::string mesh;
   std::string shader;
   std::string texture;
   std::string collision_mesh;
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

   Shader*        default_shader;
   Mesh*          default_mesh;
   Texture        default_texture;
   CollisionMesh* default_c_mesh;

   std::vector<Entity*>   deletion_stack;
   std::vector<Entity*>*  entity_registry;
   std::vector<Entity*>*  checkpoints_registry;
   std::vector<Entity*>*  interactables_registry;

   World* world;
   
   // methods
   void set_default_entity_attributes(std::string mesh,std::string shader,std::string texture);
   void set_entity_registry(std::vector<Entity*>* registry);
   void set_checkpoints_registry(std::vector<Entity*>* registry);
   void set_interactables_registry(std::vector<Entity*>* registry);
   void set_world(World* world);
   void register_in_world_and_scene(Entity* entity);

   Entity* create_entity(const EntityAttributes* attrs);
   Entity* create_entity(const std::string& name, const bool load_defaults = true);
   Entity* create_entity(const bool load_defaults = false);
   Entity* copy_entity(Entity* entity);
   Entity* create_entity(
      const std::string& name,
      const std::string& mesh,
      const std::string& shader,
      const std::string& texture,
      const std::string& collision_mesh,
      const vec3 scale);
  Entity* create_editor_entity(
      const std::string& name,
      const std::string& mesh,
      const std::string& shader,
      const std::string& texture,
      const std::string& collision_mesh,
      const vec3 scale  = vec3(1.0));

   [[nodiscard]] auto _find_entity_assets_in_catalogue(const std::string& mesh, const std::string& collision_mesh, const std::string& shader, const std::string& texture) const;
   void _remove_from_checkpoint_registry(Entity* entity) const;
   void _remove_interactivity(Entity* entity);
   void _make_interactable(Entity* entity);
   void _unset_all_type_related_configurations(Entity* entity);
   void set_type(Entity* entity, EntityType type);
   void mark_for_deletion(Entity* entity);
   void safe_delete_marked_entities();
};







