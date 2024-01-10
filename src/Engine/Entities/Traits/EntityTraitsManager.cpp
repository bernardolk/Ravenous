#include "Engine/Entities/Entity.h"
#include "EntityTraitsManager.h"

void EntityTraitsManager::Register(RTypeID TypeId, RTraitID TraitId, UpdateFuncPtr Func)
{
	if (!Find(TraitUpdateFunctions, TraitId)) {
		TraitUpdateFunctions[TraitId] = map<RTypeID, UpdateFuncPtr>();
	}
	
	if (auto* EntityTraitList = Find(EntityTraitsLists, TypeId)) {
		EntityTraitList->push_back(TypeId);
	}
	else {
		EntityTraitsLists[TypeId] = vector<RTraitID>{TypeId};
	}

	auto& EntityUpdateFunctions = TraitUpdateFunctions[TraitId];
	if (Find(EntityUpdateFunctions, TypeId)) {
		EntityUpdateFunctions[TypeId] = Func;
	}
}

auto EntityTraitsManager::GetUpdateFunc(RTypeID TypeId, RTraitID TraitId) -> UpdateFuncPtr
{
	if (auto* TypeMap = Find(TraitUpdateFunctions, TraitId))
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