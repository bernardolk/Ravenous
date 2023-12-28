#pragma once

#include "engine/catalogues.h"
#include "engine/entities/Entity.h"

struct EntityType(EStaticMesh)
{
	Reflected(EStaticMesh)

	static inline constexpr uint InstanceBudget = 200;
};