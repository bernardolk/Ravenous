#pragma once

#include "engine/entities/traits/EntityTraits.h"

struct Entity(E_StaticMesh)
{
	Reflected()
	
	E_StaticMesh() { instance_budget = 200; }
};
