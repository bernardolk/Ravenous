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
   float length;
   float height;
   float width;
   vec3 tangent;
   vec3 normal;
   float inclination;
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

   void set_scale(vec3 new_scale)
   {
      scale = new_scale;
      switch(collision_geometry_type)
      {
         case COLLISION_ALIGNED_BOX:
            recalculate_collision_aabb(new_scale);
            break;
         case COLLISION_ALIGNED_SLOPE:
            collision_geometry.slope.width   = new_scale.z;
            collision_geometry.slope.height  = new_scale.y;
            collision_geometry.slope.length  = new_scale.x;
            break;
      }
   };

   void set_slope_properties()
   {
      auto& slope = collision_geometry.slope;
      float inclination = slope.height / slope.length;
      float slope_angle = atan(inclination);
      float complementary = 180.0f - (slope_angle + 90.0f); 

      // slope geometry is defined as default (rotation = 0) being going down along +x
      // here we set the tangent vector to the slope, so the player falls along it when sliding
      auto slope_direction = vec3(0, -1 * sin(slope_angle), 0);
      auto slope_normal = vec3(0, sin(complementary), 0);
      switch((int) rotation.y)
      {
         case 0:
            slope_direction.x = cos(slope_angle);
            slope_normal.x = cos(complementary);
            break;
         case 90:
            slope_direction.z = -1 * cos(slope_angle);
            slope_normal.z = -1 * cos(complementary);
            break;
         case 180:
            slope_direction.x = -1 * cos(slope_angle);
            slope_normal.x = -1 * cos(complementary);
            break;
         case 270:
            slope_direction.z = cos(slope_angle);
            slope_normal.z = cos(complementary);
            break;
      }
      slope.tangent = slope_direction;
      slope.normal = slope_normal;
      slope.inclination = inclination;
   }

   void rotate_y(float angle)
   {
      rotation.y += angle;
      rotation.y = (int) rotation.y % 360;
      if(rotation.y < 0)
         rotation.y = 360 + rotation.y;

      switch(collision_geometry_type)
      {
         case COLLISION_ALIGNED_BOX:
            recalculate_collision_aabb(scale);
            break;
         case COLLISION_ALIGNED_SLOPE:
            set_slope_properties();
            break;
      }
   }

   void recalculate_collision_aabb(vec3 new_scale)
   {
      // Essentially, we change the lengths from local to world coordinates
      // and calculate the bounds in world coordinates in order, axis-aligned
      mat4 rot = glm::rotate(mat4identity, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
      vec3 s_world = rot * vec4(new_scale, 1.0);

      auto &bounds = collision_geometry.aabb;
      bounds.x0 = min(position.x, position.x + s_world.x);
      bounds.x1 = max(position.x, position.x + s_world.x);
      bounds.z0 = min(position.z, position.z + s_world.z);
      bounds.z1 = max(position.z, position.z + s_world.z);
      bounds.height = new_scale.y;
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
   entity_2->set_scale(entity_2->scale);
   return entity_2;
}

int get_entity_position(Scene* scene, Entity* entity)
{
   for(int i = 0; i < scene->entities.size() ; i++)
      if(scene->entities[i]->name == entity->name)
         return i;
   return -1;
}

