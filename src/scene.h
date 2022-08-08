#pragma once

#include <engine/lights.h>

struct ProgramConfig;

struct Scene {
	std::vector<Entity*>             entities;
	std::vector<SpotLight>           spotLights;
	std::vector<DirectionalLight>    directionalLights;
	std::vector<PointLight>          pointLights;
   std::vector<Entity*>             interactables;
   std::vector<Entity*>             checkpoints;

   float global_shininess        = 17;
   vec3  ambient_light           = vec3(1);
   float ambient_intensity       = 0;

   bool search_name(std::string name)
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
      ambient_light        = configs.ambient_light;
      ambient_intensity    = configs.ambient_intensity;
   }
};