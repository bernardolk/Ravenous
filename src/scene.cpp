#include "scene.h"
#include "engine/entities/entity.h"
#include "engine/rvn.h"

bool Scene::SearchName(std::string name)
{
	for(int i = 0; i < entities.size(); i++)
		if(entities[i]->name == name)
			return true;
	return false;
}

int Scene::EntityIndex(Entity* entity)
{
	for(int i = 0; i < entities.size(); i++)
		if(entities[i]->name == entity->name)
			return i;
	return -1;
}

Entity* Scene::FindEntity(std::string name)
{
	for(int i = 0; i < entities.size(); i++)
		if(entities[i]->name == name)
			return entities[i];
	return nullptr;
}

void Scene::ReloadGlobalConfigs()
{
	auto* configs = ProgramConfig::Get();
	ambient_light = configs->ambient_light;
	ambient_intensity = configs->ambient_intensity;
}