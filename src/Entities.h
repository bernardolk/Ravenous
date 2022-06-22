#pragma once

// forward declarations
struct WorldCell;

const static size_t ENTITY_WOLRD_CELL_OCCUPATION_LIMIT = 50;
const static std::string DEFAULT_ENTITY_SHADER = "model";
const static std::string ENTITY_SHADER_MARKING = "color";


enum EntityTimerTargetType {
   EntityTimerTargetType_NotATarget          = 0,
   EntityTimerTargetType_VerticalSlidingDoor = 1,
};

enum EntityType {
   EntityType_Static                   = 0,
   EntityType_Checkpoint               = 1,
   EntityType_TimerTrigger             =  2,
   EntityType_TimerTarget              = 3,
   EntityType_TimerMarking             = 4,
};

enum EntityFlags {
   EntityFlags_EmptyEntity          = (1 << 0),
   EntityFlags_InvisibleEntity      = (1 << 1),
   EntityFlags_HiddenEntity         = (1 << 2),
   EntityFlags_RenderTiledTexture   = (1 << 3),
   EntityFlags_RenderWireframe      = (1 << 4),
};

struct TimerMarkingData {
   vec3 color_off = vec3(0.1, 0.1, 0.1);
   vec3 color_on  = vec3(0.1, 0.4, 0.85);
   vec3 color;
};

struct TimerTriggerData {
   Entity*              timer_target       = nullptr;
   int                  timer_duration     = 0;          // Expressed in seconds

   const static size_t  size  = 16;
   Entity*              markings[size];                  /* not perfect name, but means the lights that show
                                                           if the player is on track or not towards the timed door */
   u32                  time_checkpoints[size];
   bool                 notification_mask[size];

   TimerTriggerData() {
      For(size)
      {
         markings[i]           = nullptr;
         notification_mask[i]  = false;
         time_checkpoints[i]   = 0;
      }
   }

   void add_marking(Entity* entity, u32 time_checkpoint)
   {
      For(size)
         if(markings[i] == nullptr)
         {
            markings[i]          = entity;
            time_checkpoints[i]  = time_checkpoint;
            return;
         }

      log(LOG_WARNING, "Max number of timer markings reached when trying to add entity as one to timer trigger entity.");
   }

   void delete_marking(int i)
   {
      markings[i]             = nullptr;
      notification_mask[i]    = false;
      time_checkpoints[i]     = 0;
   }
};

struct TimerTargetData {
   EntityTimerTargetType   timer_target_type       = EntityTimerTargetType_NotATarget;
   u32                     timer_start_animation   = 0;          // animation id of anim to play when timer starts
   u32                     timer_stop_animation    = 0;          // animation id of anim to play when timer ends
};

// =======================
// >        ENTITY
// =======================
struct Entity {

   u64 id               = -1;
   std::string name     = "NONAME";
   EntityType type      = EntityType_Static;
   u32 flags            = 0;

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
   glm::quat quaternion;

   Mesh*        collision_mesh;        // static collision mesh vertex data
   Mesh         collider;              // dynamic collision mesh, obtained by multiplying static collision mesh with model matrix
   BoundingBox  bounding_box;          // computed using the collider mesh, used for fast first pass collision tests

   // collider settings
   bool slidable = false;

   WorldCell* world_cells[ENTITY_WOLRD_CELL_OCCUPATION_LIMIT];
   int world_cells_count = 0;


   // temp missile fields
   bool dodged = false;
   float inv_period_timer = 0;




   // ---------------------------
   // > event trigger settings
   // ---------------------------
   Mesh*    trigger              = nullptr;
   vec3     trigger_scale        = vec3(1.5f, 1.f, 0.f);
   vec3     trigger_pos          = vec3(0.0f);
   mat4     trigger_matModel;

   // ---------------------------
   // > entity type data
   // ---------------------------

   union {
      TimerTriggerData  timer_trigger_data;
      TimerMarkingData  timer_marking_data;
      TimerTargetData   timer_target_data;
   };

   Entity()    : timer_marking_data() {}
   ~Entity() {}

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


/*
   ENTITIES FUNCTION LIMBO -> ALL ENTITY FUNCTIONS LIVE HERE UNTIL REFACTOR IS
   FINISHED.
*/

Triangle get_triangle_for_indexed_mesh(Entity* entity, int triangle_index);

Triangle get_triangle_for_indexed_mesh(Entity* entity, int triangle_index)
{
   Mesh* mesh = entity->mesh;
   mat4 model = entity->matModel;

   return get_triangle_for_indexed_mesh(mesh, model, triangle_index);
}