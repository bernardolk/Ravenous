#include "Engine/Entities/Entity.h"
#include "EntityTraitsManager.h"

void EntityTraitsManager::Register(REntityTypeID EntityTypeID, RTraitID TraitID, UpdateFuncPtr Func)
{
	// Add entity first time it appears
	if (!Find(EntityTraitsLists, EntityTypeID)) {
		EntityTraitsLists.insert({EntityTypeID, {}});
	}
	
	auto* Traits = Find(EntityTraitsLists, EntityTypeID);
	Traits->push_back(TraitID);

	// Add trait first time it appears
	if (!Find(TraitEntities, TraitID)) {
		TraitEntities.insert({TraitID, {}});
	}

	auto* Entities = Find(TraitEntities, TraitID);
	Entities->insert({EntityTypeID, Func});
}

auto EntityTraitsManager::GetUpdateFunc(REntityTypeID TypeId, RTraitID TraitId) -> UpdateFuncPtr
{
	if (auto* TypeMap = Find(TraitEntities, TraitId))
	{
		if (auto* UpdateFunc = Find(*TypeMap, TypeId)) {
			return *UpdateFunc;
		}
	}

	return nullptr;
}

void EntityTraitsManager::InvokeUpdate(EEntity* Entity, RTraitID TraitId)
{
	if (auto* Func = GetUpdateFunc(Entity->TypeID, TraitId)) {
		Func(Entity);
	}
	else {
		printf("Invoke function not found for TypeID: %i and trait of TraitID: %i.\n", Entity->TypeID, TraitId);
	}
}

vector<RTraitID> EntityTraitsManager::GetEntityTraits(EEntity* Entity)
{
	if (auto* TraitsList = Find(EntityTraitsLists, Entity->TypeID)) {
		return *TraitsList;
	}

	return {};
}