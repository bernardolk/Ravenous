#include "interactable.h"

void TInteractable::BlockInteractions()
{
	BlockInteraction = true;
}

void TInteractable::UnblockInteractions()
{
	BlockInteraction = false;
}

bool TInteractable::AreInteractionsBlocked()
{
	return BlockInteraction;
}

void TInteractable::SetPassiveInteraction(bool Value)
{
	PassiveInteraction = Value;
}

bool TInteractable::IsInteractionPassive()
{
	return PassiveInteraction;
}

bool TInteractable::IsVolumeCollidingWithPlayer()
{
	// TODO: Obviously
	return true;
}
