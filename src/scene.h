#pragma once

#include "engine/core/core.h"
#include <engine/lights.h>

struct Scene
{
	// TODO entity storage -> Arena allocator
	vector<Entity*> entities;
	vector<SpotLight> spot_lights;
	vector<DirectionalLight> directional_lights;
	vector<PointLight> point_lights;
	vector<Entity*> interactables;
	vector<Entity*> checkpoints;

	float global_shininess = 17;
	vec3 ambient_light = vec3(1);
	float ambient_intensity = 0;

public:
	bool SearchName(std::string name);
	int EntityIndex(Entity* entity);
	Entity* FindEntity(std::string name);
	void LoadConfigs(struct ProgramConfig configs);
};
