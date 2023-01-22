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
EntityState get_entity_state(Entity* entity);
void apply_state(EntityState state);
bool compare_entity_states(EntityState state1, EntityState state2);
mat4 mat_model_from_entity_state(EntityState state);
