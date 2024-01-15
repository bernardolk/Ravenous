#pragma once
#include "Engine/Entities/EHandle.h"
#include "engine/core/core.h"

struct REntityState
{
	vec3 Position;
	vec3 Scale;
	vec3 Rotation;

	RUUID ID;
	EHandle<EEntity> Entity;

	void Apply();
};

REntityState GetEntityState(const EHandle<EEntity>& Entity);
bool AreEntityStatesEqual(const REntityState& State1, const REntityState& State2);
mat4 MatModelFromEntityState(const REntityState& State);
