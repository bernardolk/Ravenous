#include "Engine/Entities/Entity.h"
#include "EntityTraitsManager.h"

vector<RTypeID>* EntityTraitsManager::GetTypesWithTrait(RTraitID TraitID)
{
	if (Find(TraitInverseRegistry, TraitID))
	{
		return &TraitInverseRegistry[TraitID];
	}

	return nullptr;
}

void EntityTraitsManager::Register(RTypeID TypeId, RTraitID TraitId, UpdateFuncPtr Func)
{
	// registers trait if new
	if (auto It = TraitRegistry.find(TraitId); It == TraitRegistry.end())
	{
		TraitRegistry[TraitId] = map<RTypeID, UpdateFuncPtr>();
		TraitInverseRegistry[TypeId] = vector<RTypeID>();
	}

	auto& TraitsMap = TraitRegistry[TraitId];

	// registers type update call if new
	if (Find(TraitsMap, TypeId))
	{
		TraitsMap[TypeId] = Func;
	}

	if (Find(TraitInverseRegistry, TraitId))
	{
		auto* Types = &TraitInverseRegistry[TraitId];
		Types->push_back(TypeId);
	}
}

auto EntityTraitsManager::GetUpdateFunc(RTypeID TypeId, RTraitID TraitId) -> UpdateFuncPtr
{
	// perform lookup based on trait and type id and execute lambda / wrapped update call.
	if (auto It = TraitRegistry.find(TraitId); It != TraitRegistry.end())
	{
		auto& TypeMap = It->second;

		if (auto It2 = TypeMap.find(TypeId); It2 != TypeMap.end())
		{
			return It2->second;
		}
	}

	return nullptr;
}

void EntityTraitsManager::InvokeUpdate(EEntity* Entity, RTraitID TraitId)
{
	if (auto* Func = GetUpdateFunc(Entity->TypeID, TraitId))
	{
		Func(Entity);
	}
	else
	{
		printf("Invoke function not found for TypeID: %i and trait of TraitID: %i.\n", Entity->TypeID, TraitId);
	}
}
