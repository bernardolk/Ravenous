#pragma once

#include "engine/core/core.h"

struct EEntity;

struct EntityTraitsManager
{
	static EntityTraitsManager* Get()
	{
		static EntityTraitsManager Instance{};
		return &Instance;
	}
	
	using UpdateFuncPtr = void(*)(EEntity*);

	static inline constexpr uint MaxTraits = 5;

	map<RTraitID, map<RTypeID, UpdateFuncPtr> > TraitRegistry;
	map<RTypeID, vector<RTypeID> > TraitInverseRegistry;
	vector<RTraitID> EntityTraits;

	void Register(RTypeID TypeId, RTraitID TraitId, UpdateFuncPtr Func);
	void InvokeUpdate(EEntity* Entity, RTraitID TraitId);
	vector<RTypeID>* GetTypesWithTrait(RTraitID TraitID);
	UpdateFuncPtr GetUpdateFunc(RTypeID TypeId, RTraitID TraitId);

	template<typename TEntity, typename TTrait>
	void RegisterTypeAndTraitMatch();

};

template<typename TEntity, typename TTrait>
void EntityTraitsManager::RegisterTypeAndTraitMatch()
{
	printf("Registering entity of id '%i' and trait of id '%i'.\n", TEntity::GetTypeId(), TTrait::trait_id);

	Register(TEntity::GetTypeId(), TTrait::trait_id,
		[](EEntity* InEntity) {
			auto* FullyCastEntity = static_cast<TEntity*>(InEntity);
			TTrait::Update(*FullyCastEntity);
		}
	);

	TEntity::traits.Add(TTrait::trait_id);
}
