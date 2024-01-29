#pragma once

#include "engine/entities/Entity.h"
#include "Traits/interactable.h"

struct EntityType(ECheckpoint, TInteractable)
{
	Reflected(ECheckpoint)
	// static inline constexpr uint InstanceBudget = 100;
	
	void Interact();
	vec3 GetPlayerSpawnPosition();
};
