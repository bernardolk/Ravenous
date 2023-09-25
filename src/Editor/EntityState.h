#pragma once
#include "engine/core/core.h"

struct REntityState
{
	EEntity* Entity = nullptr;
	uint64 ID;
	vec3 Position;
	vec3 Scale;
	vec3 Rotation;
};

//todo Refactor into methods
REntityState GetEntityState(EEntity* Entity);
void ApplyState(REntityState State);
bool CompareEntityStates(REntityState State1, REntityState State2);
mat4 MatModelFromEntityState(REntityState State);
