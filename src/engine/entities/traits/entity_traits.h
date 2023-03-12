#pragma once

#include "engine/core/core.h"
#include "engine/entities/manager/entity_traits_manager.h"

#define Trait(Name) Name : EntityTraitInterface<Name>

static int EntityInterfaceIDCounter = 0;

template<typename T_Entity, typename T_Trait>
char RegisterTypeAndInterfaceMatch()
{
	// register match in update dispatcher
	T_Entity entity;
	std::cout << "Registering entity: " << T_Entity::GetTypeId() << " and interface: " << T_Trait::interface_id << "\n";

	auto* etm = EntityTraitsManager::Get();
	etm->Register(T_Entity::GetTypeId(), T_Trait::interface_id, 
	[](E_BaseEntity* in_entity)
		{
			auto* cast_entity = static_cast<T_Entity*>(in_entity);
			T_Trait::Update(*cast_entity);
		}
	);

	// add trait to type
	T_Entity::traits |= T_Trait::interface_id;

	return 0;
};

template<typename T_Trait>
struct EntityTraitInterface
{
	static inline int interface_id;

	// ReSharper disable once CppFunctionIsNotImplemented
	template<typename T_Entity>
	static void Update(T_Entity& entity);

	EntityTraitInterface() 
	{  
		static int tmp_interface_id = []()
		{  
			auto tmp_trait_id = ++EntityInterfaceIDCounter;
			auto* etm = EntityTraitsManager::Get();
			etm->entity_traits.push_back(tmp_trait_id);
			return tmp_trait_id;
		}(); 
		interface_id = tmp_interface_id;
	};

};

static int TypeIDCounter = 0;

template<typename T_Entity, typename T_Trait>
struct T_TraitHelper : T_Trait
{
	static inline char _  = RegisterTypeAndInterfaceMatch<T_Entity, T_Trait>();
	static_assert(&_);     // force templated static member to be initialized
};

template<typename T_Entity>
struct T_EntityTypeHelper
{
	static inline int traits = 0;
	static inline unsigned int memory_budget = 10;
private:
	static inline int TYPE_ID;

public:
	// sets the type_id at construction
	T_EntityTypeHelper()
	{ 
		static int type_id = ++TypeIDCounter;
		TYPE_ID = type_id;
		static_cast<E_BaseEntity*>(this)->type_id = TYPE_ID;
	};

	static unsigned int GetTypeId() { return TYPE_ID; };
};