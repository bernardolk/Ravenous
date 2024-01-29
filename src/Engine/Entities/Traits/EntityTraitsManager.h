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

	vector<RTraitID> TraitsRegistry;
	map<RTraitID, map<REntityTypeID, UpdateFuncPtr>> TraitEntities;
	map<REntityTypeID, vector<RTraitID>> EntityTraitsLists; 

	void Register(REntityTypeID EntityTypeID, RTraitID TraitID, UpdateFuncPtr Func);
	void InvokeUpdate(EEntity* Entity, RTraitID TraitId);
	UpdateFuncPtr GetUpdateFunc(REntityTypeID TypeId, RTraitID TraitId);

	template<typename TEntity, typename TTrait>
	void RegisterTypeAndTraitMatch();

	vector<RTraitID> GetEntityTraits(EEntity* Entity);
};

template<typename TEntity, typename TTrait>
void EntityTraitsManager::RegisterTypeAndTraitMatch()
{
	auto EntityTypeID = TEntity::GetTypeID();
	auto TraitID = TTrait::TraitID;
	
	printf("Registering entity of id '%i' and trait of id '%i'.\n", EntityTypeID, TraitID);

	Register(EntityTypeID, TraitID,
		[](EEntity* InEntity) {
			auto* FullyCastEntity = static_cast<TEntity*>(InEntity);
			TTrait::Update(*FullyCastEntity);
		}
	);

	TEntity::Traits.Add(TraitID);
}
