#pragma once

#include <engine/lights.h>

struct ProgramConfig;

struct Scene
{
	std::vector<Entity*> entities;
	std::vector<SpotLight> spot_lights;
	std::vector<DirectionalLight> directional_lights;
	std::vector<PointLight> point_lights;
	std::vector<Entity*> interactables;
	std::vector<Entity*> checkpoints;

	float global_shininess = 17;
	vec3 ambient_light = vec3(1);
	float ambient_intensity = 0;

	bool SearchName(std::string name)
	{
		for(int i = 0; i < entities.size(); i++)
			if(entities[i]->name == name)
				return true;
		return false;
	}

	int EntityIndex(Entity* entity)
	{
		for(int i = 0; i < entities.size(); i++)
			if(entities[i]->name == entity->name)
				return i;
		return -1;
	}

	Entity* FindEntity(std::string name)
	{
		for(int i = 0; i < entities.size(); i++)
			if(entities[i]->name == name)
				return entities[i];
		return nullptr;
	}

	void LoadConfigs(ProgramConfig configs)
	{
		ambient_light = configs.ambient_light;
		ambient_intensity = configs.ambient_intensity;
	}
};
