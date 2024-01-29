#include "EHandle.h"
#include "Entity.h"

RUUID GetID(REntitySlot& Slot)
{
	return Slot.Value->ID;
}
