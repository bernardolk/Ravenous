#include "EntityState.h"

#include "Reflection/Serialization.h"
#include "engine/entities/Entity.h"

REntityState GetEntityState(const EHandle<EEntity>& Entity)
{
	if (!Entity.IsValid()) return {};
	
	REntityState State;
	State.Position = Entity->Position;
	State.Scale = Entity->Scale;
	State.Rotation = Entity->Rotation;
	State.Entity = Entity;
	State.ID = Entity->ID;
	return State;
}

void REntityState::Apply()
{
	if (!Entity.IsValid()) return;
	
	Entity->Position = Position;
	Entity->Scale = Scale;
	Entity->Rotation = Rotation;
	Entity->Update();
}

bool AreEntityStatesEqual(const REntityState& State1, const REntityState& State2)
{
	if (!State1.Entity.IsValid() || !State2.Entity.IsValid()) return false;
	
	return State1.ID == State2.ID
		&& State1.Position == State2.Position
		&& State1.Scale == State2.Scale
		&& State1.Rotation == State2.Rotation;
}

mat4 MatModelFromEntityState(const REntityState& State)
{
	glm::mat4 Model = translate(Mat4Identity, State.Position);
	Model = rotate(Model, glm::radians(State.Rotation.x), vec3(1.0f, 0.0f, 0.0f));
	Model = rotate(Model, glm::radians(State.Rotation.y), vec3(0.0f, 1.0f, 0.0f));
	Model = rotate(Model, glm::radians(State.Rotation.z), vec3(0.0f, 0.0f, 1.0f));
	Model = scale(Model, State.Scale);
	return Model;
}