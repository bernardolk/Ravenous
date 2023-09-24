#pragma once
#include "engine/core/core.h"

struct REntityState
{
	EEntity* entity = nullptr;
	uint64 id;
	vec3 position;
	vec3 scale;
	vec3 rotation;
};

//todo Refactor into methods
REntityState GetEntityState(EEntity* entity);
void ApplyState(REntityState state);
bool CompareEntityStates(REntityState state1, REntityState state2);
mat4 MatModelFromEntityState(REntityState state);
