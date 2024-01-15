#pragma once
#include "Engine/Core/RView.h"
#include "Engine/World/EntitySlot.h"

struct RWorld;
struct EEntity;

// ===================================
//	EHandle
// ===================================
// Wrapper for entity ptrs so that storing a reference to an entity across frames is safe.
// The way this works is that when a new entity is created in the world, we increment a generation int that keeps track of how many times that
// same slot in the entity storage container was used. If it was used more than what you tracked when you first got the entity ptr, then that
// means that you have a ptr to a slot which now contains a ptr to another entity.
// We don't use RUUIDs for this because that would make handles take up more space since UUIDs are large and also comparing them would be slower.

template<typename TEntity>
struct EHandle
{
	// Friends
	template<typename T> friend EHandle<T> MakeHandle(EEntity* Entity);
	template<typename T> EHandle<T> friend SpawnEntity();
	template<typename T> friend void DeleteEntity(EHandle<T> Entity);
	template<typename T> friend struct EHandle;														// Necessary to provide the converting constructor access to private members of the EHandle being converted 
	friend EHandle<EEntity> MakeHandleFromID(RUUID ID);
	
public:
	TEntity* operator->()
	{
		return IsValid() ? (TEntity*) Slot->Value : nullptr; 
	}

	const TEntity* operator->() const
	{
		return IsValid() ? (TEntity*) Slot->Value : nullptr; 
	}

	TEntity* operator*()
	{
		return IsValid() ? (TEntity*) Slot->Value : nullptr;
	}

	[[nodiscard]] bool IsValid() const
	{
		return Slot.Get() && Slot->Generation == Generation;
	}

	EHandle() = default;

	// Allows conversion of Handles of child entities to Handles of EEntity
	template <typename TChildEntity, std::enable_if_t<std::is_base_of_v<TEntity, TChildEntity>, int> = 0>
	EHandle(EHandle<TChildEntity> InHandle) : Slot(InHandle.Slot), Generation(InHandle.Generation) {}
	
private:
	EHandle(vector<REntitySlot>& Collection, REntitySlot& Slot, int Generation) : Slot(&Collection, &Slot), Generation(Generation) {}
	RView<REntitySlot> Slot;
	int Generation = -1;
};
















