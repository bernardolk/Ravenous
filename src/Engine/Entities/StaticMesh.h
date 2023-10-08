#pragma once

#include "engine/catalogues.h"
#include "engine/entities/Entity.h"

struct EntityType(EStaticMesh)
{
	Reflected()

	static inline constexpr uint InstanceBudget = 200;
};

struct Parent
{
	string Name;
	RTypeID TypeID;
};

struct EDummy : Parent, TEntityTypeBase<EDummy>
{
	
};

void AreYouKiddingMe(EEntity* Entity);