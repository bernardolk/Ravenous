#pragma once
// ReSharper disable CppFunctionIsNotImplemented

#include "engine/core/core.h"
#include "engine/entities/traits/entity_traits.h"
#include "game/entities/player.h"

/*
 * Trait that allows entities to be interacted with via action key or touching their trigger volume.
 */

struct Trait(I_Interactable)
{
	void BlockInteractions();
	void UnblockInteractions();
	bool AreInteractionsBlocked();
	void SetPassiveInteraction(bool value);
	bool IsInteractionPassive();
	
private:
	bool block_interaction = false;
	bool passive_interaction = false;

protected:
	void Interact();

	struct Cylinder
	{
		float radius;
		struct 
		{
			float x, y, z;
		} position;
	} collision_volume{};

	bool IsVolumeCollidingWithPlayer();

	public:
	template<typename T_Entity>
	static void Update(T_Entity& entity)
	{
		if (!entity.block_interaction)
		{
			if (entity.passive_interaction || Player::Get()->interact_btn)
			{
				if (entity.IsVolumeCollidingWithPlayer())
				{
					entity.Interact();
				}
			}
		}
	}
};