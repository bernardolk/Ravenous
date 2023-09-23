#pragma once

#include "engine/core/core.h"
#include "engine/entities/EEntity.h"

struct EntityTraitsManager
{
	using byte = char;
	using UpdateFunc = void(*)(E_Entity*);

	map<TraitID, map<TypeID, UpdateFunc>> trait_registry;
	map<TypeID, vector<TypeID>> trait_inverse_registry;
	vector<TraitID> entity_traits;
	static inline constexpr u32 max_traits = 5;
	
	static EntityTraitsManager* Get()
	{
		static EntityTraitsManager instance;
		return &instance;
	}

	vector<TypeID>* GetTypesWithTrait(TraitID trait_id)
	{
		if(Find(trait_inverse_registry, trait_id))
		{
			return &trait_inverse_registry[trait_id];
		}

		return nullptr;
	}

	void Register(TypeID type_id, TraitID trait_id, UpdateFunc func)
	{
		// registers trait if new
		if(auto it = trait_registry.find(trait_id); it == trait_registry.end())
		{
			trait_registry[trait_id] = map<TypeID, UpdateFunc>();
			trait_inverse_registry[type_id] = vector<TypeID>();
		}
        
		auto& traits_map = trait_registry[trait_id];

		// registers type update call if new
		if(Find(traits_map, type_id))
		{
			traits_map[type_id] = func;
		}

		if(Find(trait_inverse_registry, trait_id))
		{
			auto* types = &trait_inverse_registry[trait_id];
			types->push_back(type_id);
		}
	}

	UpdateFunc GetUpdateFunc(TypeID type_id, TraitID trait_id)
	{
		// perform lookup based on trait and type id and execute lambda / wrapped update call.
		if(auto it = trait_registry.find(trait_id); it != trait_registry.end())
		{
			auto& type_map = it->second;

			if(auto it2 = type_map.find(type_id); it2 != type_map.end())
			{
				return it2->second;
			}
		}

		return nullptr;
	}

	void InvokeUpdate(E_Entity* entity, TraitID trait_id)
	{
		if(auto* func = GetUpdateFunc(entity->type_id, trait_id))
		{
			func(entity);
		}
		else
		{
			printf("Invoke function not found for TypeID: %i and trait of TraitID: %i.\n", entity->type_id, trait_id);
		}
	}

	template<typename T_Entity, typename T_Trait>
	byte RegisterTypeAndTraitMatch()
	{
		printf("Registering entity of id '%i' and trait of id '%i'.\n", T_Entity::GetTypeId(), T_Trait::trait_id);

		Register(T_Entity::GetTypeId(), T_Trait::trait_id, 
		[](E_Entity* in_entity)
			{
				auto* fully_cast_entity = static_cast<T_Entity*>(in_entity);
				T_Trait::Update(*fully_cast_entity);
			}
		);

		T_Entity::traits.Add(T_Trait::trait_id);

		return 0;
	};
	
private:
	EntityTraitsManager(){};
};
