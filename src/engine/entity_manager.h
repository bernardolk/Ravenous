#pragma once

struct World;
struct CollisionMesh;
struct EntityPool;
struct GlobalSceneInfo;
struct EntityPool;
struct Texture;

extern GlobalSceneInfo G_SCENE_INFO;

struct EntityAttributes {
   std::string name                = "NONAME";
   std::string mesh                = "aabb";
   std::string shader              = "model";
   std::string texture             = "grey";
   std::string collision_mesh      = "aabb";
   EntityType type                 = EntityType_Static;
   vec3 scale                      = vec3{1.0f};
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
   void set_entity_registry            (std::vector<Entity*>* registry);
   void set_checkpoints_registry       (std::vector<Entity*>* registry);
   void set_interactables_registry     (std::vector<Entity*>* registry);
   void set_world                      (World* world);
   void register_in_world_and_scene    (Entity* entity) const;

   Entity* create_editor_entity        (const EntityAttributes& attrs);
   Entity* create_entity               (const EntityAttributes& attrs);
   Entity* copy_entity                 (Entity* entity);
   

   [[nodiscard]] auto _find_entity_assets_in_catalogue(
      const std::string& mesh,
      const std::string& collision_mesh,
      const std::string& shader,
      const std::string& texture) const;
   
   void _remove_from_checkpoint_registry        (Entity* entity) const;
   void _remove_interactivity                   (Entity* entity);
   void _make_interactable                      (Entity* entity);
   void _unset_all_type_related_configurations  (Entity* entity);
   void set_type                                (Entity* entity, EntityType type);
   void mark_for_deletion                       (Entity* entity);
   void safe_delete_marked_entities             ();
};







