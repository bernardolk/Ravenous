#pragma once

// forward declarations
struct WorldCell;

const static size_t ENTITY_WOLRD_CELL_OCCUPATION_LIMIT = 50;

enum EntityTimerTargetType {
   EntityTimerTargetType_NotATarget          = 0,
   EntityTimerTargetType_VerticalSlidingDoor = 1,
};

enum EntityType {
   EntityType_Static            = 0,
   EntityType_Checkpoint        = 1,
   EntityType_TimerTrigger             = 2,
};

enum EntityFlags {
   EntityFlags_EmptyEntity          = (1 << 0),
   EntityFlags_InvisibleEntity      = (1 << 1),
   EntityFlags_HiddenEntity         = (1 << 2),
   EntityFlags_RenderTiledTexture   = (1 << 3),
   EntityFlags_RenderWireframe      = (1 << 4)
};

// =======================
// >        ENTITY
// =======================
struct Entity {

   u64 id            = -1;
   string name       = "NONAME";
   EntityType type   = EntityType_Static;
   u32 flags;

   // ---------------------------
   //  > render data
   // ---------------------------
	Shader* shader;
   Mesh* mesh;
   std::vector<Texture> textures;
	glm::mat4 matModel   = mat4identity;
   
   // box UV tile setting
   int uv_tile_wrap[6] = {1,1,1,1,1,1};


   // simulation data
	vec3     position        = vec3(0.0f);
	vec3     rotation        = vec3(0.0f);
	vec3     scale           = vec3(1.0f);
   vec3     velocity        = vec3(0.0f);

   Mesh*        collision_mesh;        // static collision mesh vertex data
   Mesh         collider;              // dynamic collision mesh, obtained by multiplying static collision mesh with model matrix
   BoundingBox  bounding_box;          // computed using the collider mesh, used for fast first pass collision tests

   // collider settings
   bool slidable = false;

   WorldCell* world_cells[ENTITY_WOLRD_CELL_OCCUPATION_LIMIT];
   int world_cells_count = 0;

   // ---------------------------
   // > event trigger settings
   // ---------------------------
   Mesh*    trigger              = nullptr;
   vec3     trigger_scale        = vec3(1.5f, 1.f, 0.f);
   vec3     trigger_pos          = vec3(0.0f);
   mat4     trigger_matModel;

   // ---------------------------
   // > timer variables
   // ---------------------------
   Entity*                 timer_target       = nullptr;     /* If the entity is interactable and has a timer target,
                                                                this points to the target entity. */
   int                     timer_duration     = 0;           // Expressed in seconds
   bool                    is_timer_target    = false;       // If this entity is a target of another interactable
   EntityTimerTargetType   timer_target_type  = EntityTimerTargetType_NotATarget;
                                                             // The type of target this entity is, if it is a target of another interactable.
   std::string timer_start_animation  = "";       // if is timer target, animation to play when timer starts
   std::string timer_stop_animation   = "";       // if is timer target, animation to play when timer ends

   // ---------------------------
   // > methods
   // ---------------------------

   void update()
   {
      // @todo WE DON'T NEED TO RUN THIS EVERY TICK!
      // just run for entities that change it's entity state

      // order here is very important
      update_model_matrix();
      update_collider();
      update_bounding_box();

      if(is_interactable())
         update_trigger();  
   }


   void update_collider()
   {
      // empty collider
      collider.vertices.clear();

      // multiplies model matrix to collision mesh
      for (int i = 0; i < collision_mesh->vertices.size(); i++)
         collider.vertices.push_back(Vertex{collision_mesh->vertices[i] * matModel});
   }


   void update_model_matrix()
   {
      glm::mat4 model = translate(mat4identity, position);
		model = rotate(model, glm::radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, glm::radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, scale);
		matModel = model;
   }


   void update_bounding_box()
   {
      // uses the collider to compute an AABB 
      bounding_box = collider.compute_bounding_box();
   }

   void update_trigger()
   {
      auto centroid = bounding_box.get_centroid();
      glm::mat4 model = translate(mat4identity, centroid);

      // to avoid elipsoids
      trigger_scale.z = trigger_scale.x;
		model = glm::scale(model, trigger_scale);

		trigger_matModel = model;
   }


   void rotate_y(float angle)
   {
      rotation.y += angle;
      rotation.y = (int) rotation.y % 360;
      if(rotation.y < 0)
         rotation.y = 360 + rotation.y;
   }

   mat4 get_rotation_matrix()
   {
      mat4 rotation_matrix;		
      rotation_matrix = rotate(mat4identity,    glm::radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
		rotation_matrix = rotate(rotation_matrix, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
		rotation_matrix = rotate(rotation_matrix, glm::radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));
      return rotation_matrix;
   }

   Mesh get_trigger_collider()
   {
      Mesh trigger_collider;

      // empty collider
      trigger_collider.vertices.clear();

      // multiplies model matrix to collision mesh
      for (int i = 0; i < trigger->vertices.size(); i++)
         trigger_collider.vertices.push_back(Vertex{trigger->vertices[i] * trigger_matModel});

      return trigger_collider;
   }

   bool is_interactable()
   {
      return type == EntityType_Checkpoint || type == EntityType_TimerTrigger;
   }
};