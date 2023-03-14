#pragma once

#include "engine/core/core.h"
#include "engine/entities/traits/entity_traits.h"
#include "game/entities/traits/interactable.h"

// struct EntityDecl(E_Door, I_Interactable)

struct E_Door : E_BaseEntity, T_EntityTypeBase<E_Door>, T_TraitMixin<E_Door, I_Interactable>
{
	string name;
	
	E_Door()
	{
		SetPassiveInteraction(true);
		instance_budget = 5;
		name = "Eduardo";
	}

	void Interact()
	{
		PRINT("Interacting with door")
	};
};

