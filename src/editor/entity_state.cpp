#include "entity_state.h"
#include "engine/entities/e_entity.h"

EntityState GetEntityState(E_Entity* entity)
{
	EntityState state;
	state.position = entity->position;
	state.scale = entity->scale;
	state.rotation = entity->rotation;
	state.entity = entity;
	state.id = entity->id;
	return state;
}

void ApplyState(EntityState state)
{
	if (state.entity == nullptr)
		return;

	state.entity->position = state.position;
	state.entity->scale = state.scale;
	state.entity->rotation = state.rotation;
	state.entity->Update();
}

bool CompareEntityStates(EntityState state1, EntityState state2)
{
	return state1.id == state2.id
	&& state1.position == state2.position
	&& state1.scale == state2.scale
	&& state1.rotation == state2.rotation;
}

mat4 MatModelFromEntityState(EntityState state)
{
	glm::mat4 model = translate(Mat4Identity, state.position);
	model = rotate(model, glm::radians(state.rotation.x), vec3(1.0f, 0.0f, 0.0f));
	model = rotate(model, glm::radians(state.rotation.y), vec3(0.0f, 1.0f, 0.0f));
	model = rotate(model, glm::radians(state.rotation.z), vec3(0.0f, 0.0f, 1.0f));
	model = scale(model, state.scale);
	return model;
}
