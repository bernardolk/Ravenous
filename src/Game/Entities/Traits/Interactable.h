#pragma once
#include "engine/core/core.h"
#include "engine/entities/traits/EntityTraits.h"
#include "game/entities/EPlayer.h"
#include "Engine/Geometry/Cylinder.h"

/*  ===========================================================================================================
 *	Interactable
 *		Trait that allows entities to be interacted with via action key or touching their trigger volume.
 *	=========================================================================================================== */

struct Trait(TInteractable)
{
	Reflected_Trait(TInteractable)
	
	REQUIRES_METHOD( void Interact() )

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
		if (!Entity.bBlockInteraction) {
			if (Entity.bPassiveInteraction || EPlayer::Get()->bInteractButton) {
				if (Entity.IsVolumeCollidingWithPlayer()) {
					Entity.Interact();
				}
			}
		}
	}

protected:
	Field(RCylinder, Cylinder);

	bool IsVolumeCollidingWithPlayer();

private:
	Field(bool, bBlockInteraction) = false;
	Field(bool, bPassiveInteraction) = false;
};
