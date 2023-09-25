#pragma once

#include "engine/entities/traits/EntityTraits.h"

struct EntityType(EStaticMesh)
{
	Reflected()

	EStaticMesh() { InstanceBudget = 200; }
};
