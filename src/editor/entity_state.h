#pragma once
#include "engine/core/core.h"

struct EntityState
{
	Entity* entity = nullptr;
	u64 id;
	vec3 position;
	vec3 scale;
	vec3 rotation;
};

//todo Refactor into methods
EntityState GetEntityState(Entity* entity);
void ApplyState(EntityState state);
bool CompareEntityStates(EntityState state1, EntityState state2);
mat4 MatModelFromEntityState(EntityState state);
