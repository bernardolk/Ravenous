#pragma once

#include "engine/entities/traits/entity_traits.h"

struct Entity(E_StaticMesh)
{
	Reflected()
	
	E_StaticMesh() { instance_budget = 200; }
};
