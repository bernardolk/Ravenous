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
   float length_x;
   float length_y;
   float length_z;
};

struct CollisionGeometrySlope{
   float slope_length;
   float slope_height;
   float slope_width;
   glm::vec3 tangent;
   glm::vec3 normal;
   float inclination;
};

struct Entity {
   // administrative data
	unsigned int index;
	unsigned int id;
   std::string name = "NONAME";

   // render data
	Shader* shader;
   GLData gl_data;
   Mesh mesh;
   std::vector<Texture> textures;
	glm::mat4 matModel = mat4identity;
   bool render_me = true;

   // simulation data
	glm::vec3 position;
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
   glm::vec3 velocity;

   // collision simulation data
   void* collision_geometry_ptr;
   CollisionGeometryEnum collision_geometry_type;
};

struct SpotLight {
	unsigned int id;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 ambient;
	float innercone;
	float outercone;
	float intensity_constant = 0.02f;
	float intensity_linear = 1.0f;
	float intensity_quadratic = 0.032f;
};

struct PointLight {
	unsigned int id;
	glm::vec3 position = glm::vec3(0.0f, 2.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 ambient = glm::vec3(0.01f, 0.01f, 0.01f);
	float intensity_constant = 1.0f;
	float intensity_linear = 0.5f;
	float intensity_quadratic = 0.1f;
};

struct DirectionalLight {
	unsigned int id;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 ambient;
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
   {
      if(scene->entities[i]->name == name)
      {
         return scene->entities[i];
      }
   }

   return NULL;
}