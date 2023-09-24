#include "EntityTraitsManager.h"

EntityTraitsManager::EntityTraitsManager() = default;


vector<TypeID>* EntityTraitsManager::GetTypesWithTrait(TraitID trait_id)
{
	if(Find(trait_inverse_registry, trait_id))
	{
		return &trait_inverse_registry[trait_id];
	}

	return nullptr;
}

void EntityTraitsManager::Register(TypeID type_id, TraitID trait_id, UpdateFuncPtr func)
{
	// registers trait if new
	if(auto it = trait_registry.find(trait_id); it == trait_registry.end())
	{
		trait_registry[trait_id] = map<TypeID, UpdateFuncPtr>();
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

auto EntityTraitsManager::GetUpdateFunc(TypeID type_id, TraitID trait_id) -> UpdateFuncPtr
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

void EntityTraitsManager::InvokeUpdate(EEntity* entity, TraitID trait_id)
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