#pragma once
#include "Engine/Core/Core.h"

struct REntitySlot
{
	friend RWorld;
	friend struct REntityStorage;
	
	EEntity* Value = nullptr;
	int Generation = 1;

private:
	// Only the world should instantiate slots, only ptrs to them should be used by other systems.
	// This is because then we can easily get the offset of that slot from the list of slots using ptr arithmetic. That avoids iterating through the whole list of slots to find the one we have stored in an entity handle (for example).
	// Also, for correctness, because copying a slot means copying an entity ptr instead of making / storing a handle to the entity.
	REntitySlot() = default;
};

struct REntityStorage
{
	vector<REntitySlot> EntitySlots;
	vector<REntitySlot*> EmptySlots;

	REntitySlot* Add(EEntity* Entity);
	void Empty(REntitySlot* Slot);

	REntityStorage()
	{
		EntitySlots.reserve(100);
	}

	bool SlotContainsEntity(const REntitySlot* Slot) const
	{
		for (auto* EmptySlot : EmptySlots) {
			if (EmptySlot == Slot) {
				return false;
			}
		}
		return true;
	}
};
