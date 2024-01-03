#pragma once
#include "engine/core/core.h"
#include "engine/entities/traits/EntityTraits.h"
#include "game/entities/EPlayer.h"

/*
 * Trait that allows entities to be interacted with via action key or touching their trigger volume.
 */

struct Trait(TInteractable)
{
	REQUIRES_METHOD(void Interact())

	void BlockInteractions();
	void UnblockInteractions();
	bool IsInteractionBlocked();
	void SetPassiveInteraction(bool Value);
	bool IsInteractionPassive();

   /* ========================================
	* Update
	* ======================================== */	
	
	template<typename T>
	static void Update(T& Entity)
	{
		if (!Entity.bBlockInteraction)
		{
			if (Entity.bPassiveInteraction || EPlayer::Get()->bInteractButton)
			{
				if (Entity.IsVolumeCollidingWithPlayer())
				{
					Entity.Interact();
				}
			}
		}
	}

protected:
	struct Cylinder
	{
		float Radius = 0.f;
		struct
		{
			float X = 0.f;
			float Y = 0.f;
			float Z = 0.f;
		} Position;
	} CollisionVolume;

	bool IsVolumeCollidingWithPlayer();

private:
	bool bBlockInteraction = false;
	bool bPassiveInteraction = false;
};
