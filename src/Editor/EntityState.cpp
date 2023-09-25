#include "EntityState.h"
#include "engine/entities/Entity.h"

REntityState GetEntityState(EEntity* Entity)
{
	REntityState State;
	State.Position = Entity->Position;
	State.Scale = Entity->Scale;
	State.Rotation = Entity->Rotation;
	State.Entity = Entity;
	State.ID = Entity->ID;
	return State;
}

void ApplyState(REntityState State)
{
	if (State.Entity == nullptr)
		return;

	State.Entity->Position = State.Position;
	State.Entity->Scale = State.Scale;
	State.Entity->Rotation = State.Rotation;
	State.Entity->Update();
}

bool CompareEntityStates(REntityState State1, REntityState State2)
{
	return State1.ID == State2.ID
	&& State1.Position == State2.Position
	&& State1.Scale == State2.Scale
	&& State1.Rotation == State2.Rotation;
}

mat4 MatModelFromEntityState(REntityState State)
{
	glm::mat4 Model = translate(Mat4Identity, State.Position);
	Model = rotate(Model, glm::radians(State.Rotation.x), vec3(1.0f, 0.0f, 0.0f));
	Model = rotate(Model, glm::radians(State.Rotation.y), vec3(0.0f, 1.0f, 0.0f));
	Model = rotate(Model, glm::radians(State.Rotation.z), vec3(0.0f, 0.0f, 1.0f));
	Model = scale(Model, State.Scale);
	return Model;
}
