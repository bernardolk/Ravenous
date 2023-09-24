#pragma once

#include "engine/core/core.h"
#include "engine/entities/Entity.h"

struct EntityTraitsManager
{
	DeclSingleton(EntityTraitsManager)
	
	using UpdateFuncPtr = void(*)(EEntity*);
	
	static inline constexpr uint max_traits = 5;

	map<TraitID, map<TypeID, UpdateFuncPtr>> trait_registry;
	map<TypeID, vector<TypeID>> trait_inverse_registry;
	vector<TraitID> entity_traits;

	void Register(TypeID type_id, TraitID trait_id, UpdateFuncPtr func);
	void InvokeUpdate(EEntity* entity, TraitID trait_id);
	vector<TypeID>* GetTypesWithTrait(TraitID trait_id);
	UpdateFuncPtr GetUpdateFunc(TypeID type_id, TraitID trait_id);

	template<typename T_Entity, typename T_Trait>
	byte RegisterTypeAndTraitMatch();
	
};

template<typename T_Entity, typename T_Trait>
byte EntityTraitsManager::RegisterTypeAndTraitMatch()
{
	printf("Registering entity of id '%i' and trait of id '%i'.\n", T_Entity::GetTypeId(), T_Trait::trait_id);

	Register(T_Entity::GetTypeId(), T_Trait::trait_id, 
	[](EEntity* in_entity)
		{
			auto* fully_cast_entity = static_cast<T_Entity*>(in_entity);
			T_Trait::Update(*fully_cast_entity);
		}
	);

	T_Entity::traits.Add(T_Trait::trait_id);

	return 0;
}