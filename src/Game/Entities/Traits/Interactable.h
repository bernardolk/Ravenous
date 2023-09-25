#pragma once
// ReSharper disable CppFunctionIsNotImplemented

#include "engine/core/core.h"
#include "engine/entities/traits/EntityTraits.h"
#include "game/entities/EPlayer.h"

/*
 * Trait that allows entities to be interacted with via action key or touching their trigger volume.
 */

struct Trait(TInteractable)
{
	void BlockInteractions();
	void UnblockInteractions();
	bool AreInteractionsBlocked();
	void SetPassiveInteraction(bool Value);
	bool IsInteractionPassive();

private:
	bool BlockInteraction = false;
	bool PassiveInteraction = false;

protected:
	void Interact();

	struct Cylinder
	{
		float Radius;
		struct
		{
			float X, Y, Z;
		} Position;
	} CollisionVolume{};

	bool IsVolumeCollidingWithPlayer();

public:
	template<typename T_Entity>
	static void Update(T_Entity& Entity)
	{
		if (!Entity.block_interaction)
		{
			if (Entity.passive_interaction || EPlayer::Get()->interact_btn)
			{
				if (Entity.IsVolumeCollidingWithPlayer())
				{
					Entity.Interact();
				}
			}
		}
	}
};
