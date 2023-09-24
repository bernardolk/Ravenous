#pragma once

#include "engine/entities/traits/EntityTraits.h"

struct EntityType(EStaticMesh)
{
	Reflected()
	
	EStaticMesh() { instance_budget = 200; }
};
