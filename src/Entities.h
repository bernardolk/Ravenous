#pragma once

// forward declarations
struct WorldCell;

// [START] OLD TYPES - I HOPE TO GET RID OF THEM SOON D:
enum CollisionGeometryEnum {
   COLLISION_ALIGNED_CYLINDER,
   COLLISION_ALIGNED_BOX,
   COLLISION_ALIGNED_SLOPE,
};

struct CollisionGeometryAlignedCylinder {
   float half_length;
   float radius;
};

struct CollisionGeometryAlignedBox {
   float height;
   float x0;
   float x1;
   float z0;
   float z1;
};

struct CollisionGeometrySlope {
   float height;
   float x0;
   float x1;
   float z0;
   float z1;
   float inclination;
   vec3 tangent;
   vec3 normal;
};

// [END] OLD TYPES

enum EntityType {
   STATIC            = 0,
   CHECKPOINT        = 1
};

const static size_t ENTITY_WOLRD_CELL_OCCUPATION_LIMIT = 50;

struct Entity {
   u32 id;
   string name       = "NONAME";
   EntityType type   = STATIC;

   // render data
	Shader* shader;
   Mesh* mesh;
   std::vector<Texture> textures;
	glm::mat4 matModel   = mat4identity;
   bool render_me       = true;
   bool wireframe       = false;

   // simulation data
	vec3 position        = vec3(0.0f);
	vec3 rotation        = vec3(0.0f);
	vec3 scale           = vec3(1.0f);
   vec3 velocity        = vec3(0.0f);

   // collision simulation data
   CollisionGeometryEnum collision_geometry_type;
   union CollisionGeometry{
      CollisionGeometrySlope slope;
      CollisionGeometryAlignedCylinder cylinder;
      CollisionGeometryAlignedBox aabb;
   } collision_geometry;

   Mesh* collision_mesh;         // static collision mesh vertex data
   Mesh  collider;               // dynamic collision mesh, obtained by multiplying static collision mesh with model matrix
   BoundingBox bounding_box;     // computed using the collider mesh, used for fast first pass collision tests

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

   auto get_rect_bounds()
   {
      struct rect_bounds{
         float x0, x1, z0, z1;
      } bounds;

      switch(collision_geometry_type)
      {
         case COLLISION_ALIGNED_BOX:
         {
            auto aabb = collision_geometry.aabb;
            bounds.x0 = aabb.x0; bounds.x1 = aabb.x1; 
            bounds.z0 = aabb.z0; bounds.z1 = aabb.z1;
            break;
         }
         case COLLISION_ALIGNED_SLOPE:
         {
            auto slope = collision_geometry.slope;
            bounds.x0 = slope.x0; bounds.x1 = slope.x1; 
            bounds.z0 = slope.z0; bounds.z1 = slope.z1;
            break;
         }
         case COLLISION_ALIGNED_CYLINDER:
         {
            auto cyl = collision_geometry.cylinder;
            bounds.x0 = position.x - cyl.radius ; bounds.x1 = position.x + cyl.radius;
            bounds.z0 = position.z - cyl.radius ; bounds.z1 = position.z + cyl.radius;
            break;
         }
      }
      return bounds;
   }


   void update()
   {
      // @todo WE DON'T NEED TO RUN THIS EVERY TICK!
      // just run for entities that change it's entity state

      // order here is very important
      update_model_matrix();
      update_collider();
      update_bounding_box();
      old_update_collision_geometry();
      update_trigger();
   }


   void update_collider()
   {
      // empty collider
      collider.indices.clear();
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


   void old_update_collision_geometry()
   {
      // OLD CODE - HOPE TO GET RID OF IT SOON D:
      mat4 rot = glm::rotate(mat4identity, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
      vec3 s_world = rot * vec4(scale, 1.0);

      switch(collision_geometry_type)
      {
         case COLLISION_ALIGNED_BOX:
         {
            auto &bounds = collision_geometry.aabb;
            // Essentially, we change the lengths from local to world coordinates
            // and calculate the bounds in world coordinates in order, axis-aligned
            bounds.x0 = min(position.x, position.x + s_world.x);
            bounds.x1 = max(position.x, position.x + s_world.x);
            bounds.z0 = min(position.z, position.z + s_world.z);
            bounds.z1 = max(position.z, position.z + s_world.z);
            bounds.height = scale.y;
            break;
         }
         case COLLISION_ALIGNED_SLOPE:
         {
            auto &bounds = collision_geometry.slope;
            bounds.x0 = min(position.x, position.x + s_world.x);
            bounds.x1 = max(position.x, position.x + s_world.x);
            bounds.z0 = min(position.z, position.z + s_world.z);
            bounds.z1 = max(position.z, position.z + s_world.z);
            bounds.height = scale.y;

            auto& slope = collision_geometry.slope;
            // rotates slope tangent to match entity rotation
            // slope geometry is defined as default (rotation = 0) being going down along +x
            // here we set the tangent vector to the slope, so the player falls along it when sliding
            slope.inclination = scale.y / scale.x;
            float slope_angle = atan(slope.inclination);
            slope.tangent = rot * vec4(1.0f, -1 * sin(slope_angle), 0.0f, 1.0f);
            slope.normal  = rot * vec4(1.0f, sin(slope_angle), 0.0f, 1.0f);
            break;
         }
      }
   }

   void update_trigger()
   {
      auto [x0, x1, z0, z1] = get_rect_bounds();
      trigger_pos = position + vec3{(x1 - x0) / 2.0f, trigger_scale.y, (z1 - z0) / 2.0f};
      glm::mat4 model = translate(mat4identity, trigger_pos);
		model = rotate(model, glm::radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, glm::radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));
      // to avoid elipsoids
      trigger_scale.z = trigger_scale.x;
		model = glm::scale(model, trigger_scale);
		trigger_model = model;
   }


   void rotate_y(float angle)
   {
      rotation.y += angle;
      rotation.y = (int) rotation.y % 360;
      if(rotation.y < 0)
         rotation.y = 360 + rotation.y;
   }

   float get_height()
   {
       switch(collision_geometry_type)
      {
         case COLLISION_ALIGNED_BOX:
         {
            auto aabb = collision_geometry.aabb;
            return aabb.height;
         }
         case COLLISION_ALIGNED_SLOPE:
         {
            auto slope = collision_geometry.slope;
            return slope.height;
         }
         case COLLISION_ALIGNED_CYLINDER:
         {
            auto cylinder = collision_geometry.cylinder;
            return cylinder.half_length;
         }
         default: return 0;
      }
   }
};

struct EntityState {
   Entity* entity = nullptr;
   unsigned int id;
   vec3 position;
   vec3 scale;
   vec3 rotation;
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

inline
vec2 get_slope_normal(Entity* slope)
{
   auto col_geometry = slope->collision_geometry.slope;
   auto nrml = glm::normalize(vec2(col_geometry.tangent.x, col_geometry.tangent.z));
   return nrml;
}




