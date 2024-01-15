#include "EntitySlot.h"

#include "Engine/Entities/Entity.h"

REntitySlot* REntityStorage::Add(EEntity* Entity)
{
	if (EmptySlots.empty()) {
		REntitySlot Slot;
		Slot.Value = Entity;
		EntitySlots.push_back(Slot);
		return &EntitySlots.back();
	}

	auto* EmptySlot = EmptySlots.back(); EmptySlots.pop_back();
	EmptySlot->Value = Entity;
	return EmptySlot;
}

void REntityStorage::Empty(REntitySlot* Slot)
{
	Slot->Generation++;
	EmptySlots.push_back(Slot);
}