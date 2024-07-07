#pragma once

#include "engine/entities/Entity.h"
#include "Traits/Interactable.h"

struct EntityType(ECheckpoint, TInteractable)
{
	Reflected(ECheckpoint)
	// static inline constexpr uint InstanceBudget = 100;

	ECheckpoint();
	
	void Interact();
	vec3 GetPlayerSpawnPosition();
};
