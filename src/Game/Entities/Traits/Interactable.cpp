#include "interactable.h"

void I_Interactable::BlockInteractions()
{
	block_interaction = true;
}

void I_Interactable::UnblockInteractions()
{
	block_interaction = false;
}

bool I_Interactable::AreInteractionsBlocked()
{
	return block_interaction;
}

void I_Interactable::SetPassiveInteraction(bool value)
{
	passive_interaction = value;
}

bool I_Interactable::IsInteractionPassive()
{
	return passive_interaction;
}

bool I_Interactable::IsVolumeCollidingWithPlayer()
{
	// TODO: Obviously
	return true;
}