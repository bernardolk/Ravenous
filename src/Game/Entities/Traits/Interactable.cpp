#include "interactable.h"

void TInteractable::BlockInteractions()
{
	block_interaction = true;
}

void TInteractable::UnblockInteractions()
{
	block_interaction = false;
}

bool TInteractable::AreInteractionsBlocked()
{
	return block_interaction;
}

void TInteractable::SetPassiveInteraction(bool value)
{
	passive_interaction = value;
}

bool TInteractable::IsInteractionPassive()
{
	return passive_interaction;
}

bool TInteractable::IsVolumeCollidingWithPlayer()
{
	// TODO: Obviously
	return true;
}