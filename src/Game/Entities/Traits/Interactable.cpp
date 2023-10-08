#include "interactable.h"

void TInteractable::BlockInteractions()
{
	bBlockInteraction = true;
}

void TInteractable::UnblockInteractions()
{
	bBlockInteraction = false;
}

bool TInteractable::IsInteractionBlocked()
{
	return bBlockInteraction;
}

void TInteractable::SetPassiveInteraction(bool Value)
{
	bPassiveInteraction = Value;
}

bool TInteractable::IsInteractionPassive()
{
	return bPassiveInteraction;
}

bool TInteractable::IsVolumeCollidingWithPlayer()
{
	// TODO: Obviously, place code to check if is colliding with player
	return true;
}
