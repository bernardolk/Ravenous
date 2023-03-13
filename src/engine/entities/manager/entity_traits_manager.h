#pragma once

#include "engine/core/core.h"
#include "engine/entities/base_entity.h"

struct EntityTraitsManager
{
	using byte = char;
	using UpdateFunc = void(*)(E_BaseEntity*);

	map<TraitID, map<TypeID, UpdateFunc>> trait_registry;
	vector<TraitID> entity_traits;
	static inline constexpr size_t max_traits = 5; 
	
	static EntityTraitsManager* Get()
	{
		static EntityTraitsManager instance;
		return &instance;
	}

	void Register(TypeID type_id, TraitID trait_id, UpdateFunc func)
	{
		// registers trait if new
		if(auto it = trait_registry.find(trait_id); it == trait_registry.end())
		{
			trait_registry[trait_id] = std::map<TypeID, UpdateFunc>();
		}
        
		auto& i_map = trait_registry[trait_id];
        
		// registers type update call if new
		if(auto it = i_map.find(type_id); it == i_map.end())
		{
			i_map[type_id] = func;
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

	void InvokeUpdate(E_BaseEntity* entity, TraitID trait_id)
	{
		if(auto* func = GetUpdateFunc(entity->type_id, trait_id))
		{
			func(entity);
		}
		else
		{
			std::cout << "INVOKE FUNCTION NOT FOUND FOR ENTITY OF TypeID: " << entity->type_id <<  " AND TRAIT of TraitID: " << trait_id << "\n";
		}
	}

	template<typename T_Entity, typename T_Trait>
	byte RegisterTypeAndTraitMatch()
	{
		std::cout << "Registering entity of id " << T_Entity::GetTypeId() << " and trait of id " << T_Trait::trait_id << "\n";

		Register(T_Entity::GetTypeId(), T_Trait::trait_id, 
		[](E_BaseEntity* in_entity)
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
