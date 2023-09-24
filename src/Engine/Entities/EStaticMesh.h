#pragma once

#include "engine/entities/traits/EntityTraits.h"

struct Entity(EStaticMesh)
{
	Reflected()
	
	EStaticMesh() { instance_budget = 200; }
};
