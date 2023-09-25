#pragma once

#include "engine/core/core.h"
#include "engine/entities/Entity.h"

struct EntityTraitsManager
{
	DeclSingleton(EntityTraitsManager)
	using UpdateFuncPtr = void(*)(EEntity*);

	static inline constexpr uint MaxTraits = 5;

	map<TraitID, map<TypeID, UpdateFuncPtr> > TraitRegistry;
	map<TypeID, vector<TypeID> > TraitInverseRegistry;
	vector<TraitID> EntityTraits;

	void Register(TypeID TypeId, TraitID TraitId, UpdateFuncPtr Func);
	void InvokeUpdate(EEntity* Entity, TraitID TraitId);
	vector<TypeID>* GetTypesWithTrait(TraitID TraitID);
	UpdateFuncPtr GetUpdateFunc(TypeID TypeId, TraitID TraitId);

	template<typename TEntity, typename TTrait>
	byte RegisterTypeAndTraitMatch();

};

template<typename TEntity, typename TTrait>
byte EntityTraitsManager::RegisterTypeAndTraitMatch()
{
	printf("Registering entity of id '%i' and trait of id '%i'.\n", TEntity::GetTypeId(), TTrait::trait_id);

	Register(TEntity::GetTypeId(), TTrait::trait_id,
		[](EEntity* InEntity) {
			auto* FullyCastEntity = static_cast<TEntity*>(InEntity);
			TTrait::Update(*FullyCastEntity);
		}
	);

	TEntity::traits.Add(TTrait::trait_id);

	return 0;
}
