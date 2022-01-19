#pragma once

// forward declarations
struct WorldCell;


enum EntityType {
   STATIC            = 0,
   CHECKPOINT        = 1
};

const static size_t ENTITY_WOLRD_CELL_OCCUPATION_LIMIT = 50;

enum EntityFlags {
   EntityFlags_EmptyEntity          = (1 << 0),
   EntityFlags_InvisibleEntity      = (1 << 1),
   EntityFlags_HiddenEntity         = (1 << 2),
   EntityFlags_RenderTiledTexture   = (1 << 3),
   EntityFlags_RenderWireframe      = (1 << 4)
};

struct Entity {
   u32 id;
   string name       = "NONAME";
   EntityType type   = STATIC;
   u32 flags;

   // render data
	Shader* shader;
   Mesh* mesh;
   std::vector<Texture> textures;
	glm::mat4 matModel   = mat4identity;
   
   // box UV tile setting
   int uv_tile_wrap[6] = {1,1,1,1,1,1};


   // simulation data
	vec3 position        = vec3(0.0f);
	vec3 rotation        = vec3(0.0f);
	vec3 scale           = vec3(1.0f);
   vec3 velocity        = vec3(0.0f);

   Mesh* collision_mesh;         // static collision mesh vertex data
   Mesh  collider;               // dynamic collision mesh, obtained by multiplying static collision mesh with model matrix
   BoundingBox bounding_box;     // computed using the collider mesh, used for fast first pass collision tests

   // collider settings
   bool slidable = false;

   WorldCell* world_cells[ENTITY_WOLRD_CELL_OCCUPATION_LIMIT];
   int world_cells_count = 0;

   // event trigger
   Mesh* trigger;
   vec3 trigger_scale   = vec3(1.5f, 1.f, 0.f);
   vec3 trigger_pos     = vec3(0.0f);
   mat4 trigger_model;


   // ----------
   // > METHODS
   // ----------

   void update()
   {
      // @todo WE DON'T NEED TO RUN THIS EVERY TICK!
      // just run for entities that change it's entity state

      // order here is very important
      update_model_matrix();
      update_collider();
      update_bounding_box();
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
      // auto [x0, x1, z0, z1] = get_rect_bounds();
      // trigger_pos = position + vec3{(x1 - x0) / 2.0f, trigger_scale.y, (z1 - z0) / 2.0f};
      // glm::mat4 model = translate(mat4identity, trigger_pos);
		// model = rotate(model, glm::radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
		// model = rotate(model, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
		// model = rotate(model, glm::radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));
      // // to avoid elipsoids
      // trigger_scale.z = trigger_scale.x;
		// model = glm::scale(model, trigger_scale);
		// trigger_model = model;

      // ???
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
};

struct SpotLight {
	vec3 position = vec3(0);
	vec3 direction = vec3(0, -1, 0);
	vec3 diffuse = vec3(1);
	vec3 specular = vec3(1);
	float innercone = 1;
	float outercone = 0.5;
	float intensity_constant = 0.02f;
	float intensity_linear = 1.0f;
	float intensity_quadratic = 0.032f;
};

struct PointLight {
	vec3 position = vec3(0);
	vec3 diffuse = vec3(1);
	vec3 specular = vec3(1);
	float intensity_constant = 0.5f;
	float intensity_linear = 0.4f;
	float intensity_quadratic = 0.032f;
};

struct DirectionalLight {
	vec3 direction = vec3(0, -1, 0);
	vec3 diffuse = vec3(1);
	vec3 specular = vec3(1);
};

struct Scene {
	std::vector<Entity*> entities;
	std::vector<SpotLight> spotLights;
	std::vector<DirectionalLight> directionalLights;
	std::vector<PointLight> pointLights;
   std::vector<Entity*> checkpoints;
   float global_shininess = 17;
   vec3 ambient_light = vec3(1);
   float ambient_intensity = 0;

   bool search_name(string name)
   {
      for(int i = 0; i < entities.size() ; i++)
         if(entities[i]->name == name)
            return true;
      return false;
   }

   int entity_index(Entity* entity)
   {
      for(int i = 0; i < entities.size() ; i++)
         if(entities[i]->name == entity->name)
            return i;
      return -1;
   }

   Entity* find_entity(std::string name) 
   {
      for(int i = 0; i < entities.size() ; i++)
         if(entities[i]->name == name)
            return entities[i];
      return NULL;
   }

   void load_configs(ProgramConfig configs)
   {
      ambient_light = configs.ambient_light;
      ambient_intensity = configs.ambient_intensity;
   }
};
