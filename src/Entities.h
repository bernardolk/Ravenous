#pragma once

struct GlobalEntityInfo {
   u32 entity_counter = 0;
};

enum CollisionGeometryEnum {
   COLLISION_ALIGNED_CYLINDER,
   COLLISION_ALIGNED_BOX,
   COLLISION_ALIGNED_SLOPE,
};

struct CollisionGeometryAlignedCylinder{
   float half_length;
   float radius;
};

struct CollisionGeometryAlignedBox{
   float height;
   float x0;
   float x1;
   float z0;
   float z1;
};

struct CollisionGeometrySlope{
   float height;
   float x0;
   float x1;
   float z0;
   float z1;
   float inclination;
   vec3 tangent;
   vec3 normal;
};

struct Entity {
   string name = "NONAME";

   // render data
	Shader* shader;
   Mesh* mesh;
   std::vector<Texture> textures;
	glm::mat4 matModel = mat4identity;
   bool render_me = true;
   bool wireframe = false;

   // simulation data
	vec3 position = vec3(0.0f);
	vec3 rotation = vec3(0.0f);
	vec3 scale = vec3(1.0f);
   vec3 velocity;

   // editor gizmo stuff (should be out of here in the future)
   vec3 gizmo_position = vec3(0.0f);

   // collision simulation data
   CollisionGeometryEnum collision_geometry_type;
   union CollisionGeometry{
      CollisionGeometrySlope slope;
      CollisionGeometryAlignedCylinder cylinder;
      CollisionGeometryAlignedBox aabb;
   } collision_geometry;

   void update_model_matrix()
   {
      glm::mat4 model = translate(mat4identity, position);
		model = rotate(model, glm::radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, glm::radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, scale);
		matModel = model;
   }

   void update_collision_geometry()
   {
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


   void rotate_y(float angle)
   {
      rotation.y += angle;
      rotation.y = (int) rotation.y % 360;
      if(rotation.y < 0)
         rotation.y = 360 + rotation.y;
   }

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
};

struct SpotLight {
	unsigned int id;
	vec3 position;
	vec3 direction;
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;
	float innercone;
	float outercone;
	float intensity_constant = 0.02f;
	float intensity_linear = 1.0f;
	float intensity_quadratic = 0.032f;
};

struct PointLight {
	vec3 position = vec3(0.0f, 2.0f, 0.0f);
	vec3 direction = vec3(0.0f, -1.0f, 0.0f);
	vec3 diffuse = vec3(0.5f, 0.5f, 0.5f);
	vec3 specular = vec3(1.0f, 1.0f, 1.0f);
	vec3 ambient = vec3(0.01f, 0.01f, 0.01f);
	float intensity_constant = 1.0f;
	float intensity_linear = 0.5f;
	float intensity_quadratic = 0.1f;
};

struct DirectionalLight {
	unsigned int id;
	vec3 position;
	vec3 direction;
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;
};

struct Scene {
	std::vector<Entity*> entities;
	std::vector<SpotLight> spotLights;
	std::vector<DirectionalLight> directionalLights;
	std::vector<PointLight> pointLights;
   float global_shininess = 32.0f;
	//vector<LightEntity> lights;
};



Entity* find_entity_in_scene(Scene* scene, std::string name) 
{
   for(int i = 0; i < scene->entities.size() ; i++)
      if(scene->entities[i]->name == name)
         return scene->entities[i];
   return NULL;
}

Entity* copy_entity(Entity* entity)
{
   auto entity_2 = new Entity();
   *entity_2 = *entity;
   return entity_2;
}

int get_entity_position(Scene* scene, Entity* entity)
{
   for(int i = 0; i < scene->entities.size() ; i++)
      if(scene->entities[i]->name == entity->name)
         return i;
   return -1;
}

