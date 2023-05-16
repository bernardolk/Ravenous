#pragma once

#include "engine/core/core.h"
#include "engine/entities/e_base_entity.h"
#include "engine/entities/manager/entity_traits_manager.h"

#ifdef WITH_EDITOR
	#define STORE_TYPE_IN_HELPER(TypeName)											template<> struct Reflection::Helper<__COUNTER__>{ using T = TypeName; };
	#define ENTITY1(TypeName)                               						TypeName; STORE_TYPE_IN_HELPER(TypeName) TypeName : E_Entity, T_EntityTypeBase<TypeName>, Reflection::ReflectionGetterSetter<TypeName>
	#define ENTITY2(TypeName, Trait1)                       						TypeName; STORE_TYPE_IN_HELPER(TypeName) TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, Reflection::ReflectionGetterSetter<TypeName, Trait1>
	#define ENTITY3(TypeName, Trait1, Trait2)               						TypeName; STORE_TYPE_IN_HELPER(TypeName) TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2>
	#define ENTITY4(TypeName, Trait1, Trait2, Trait3)								TypeName; STORE_TYPE_IN_HELPER(TypeName) TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2, Trait3>
	#define ENTITY5(TypeName, Trait1, Trait2, Trait3, Trait4)						TypeName; STORE_TYPE_IN_HELPER(TypeName) TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2, Trait3, Trait4>
	#define ENTITY6(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5)				TypeName; STORE_TYPE_IN_HELPER(TypeName) TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, T_TraitMixin<TypeName, Trait5>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2, Trait3, Trait4, Trait5>
	#define ENTITY7(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5, Trait6)       TypeName; STORE_TYPE_IN_HELPER(TypeName) TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, T_TraitMixin<TypeName, Trait5>, T_TraitMixin<TypeName, Trait6>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2, Trait3, Trait4, Trait5, Trait6>
#else
	#define ENTITY1(TypeName)                               						TypeName : E_Entity, T_EntityTypeBase<TypeName>
	#define ENTITY2(TypeName, Trait1)                       						TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>
	#define ENTITY3(TypeName, Trait1, Trait2)               						TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>
	#define ENTITY4(TypeName, Trait1, Trait2, Trait3)       						TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>
	#define ENTITY5(TypeName, Trait1, Trait2, Trait3, Trait4)						TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>
	#define ENTITY6(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5)				TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, T_TraitMixin<TypeName, Trait5>
	#define ENTITY7(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5, Trait6)       TypeName : E_Entity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, T_TraitMixin<TypeName, Trait5>, T_TraitMixin<TypeName, Trait6>
#endif

#define GET_MACRO_ENTITY(_1,_2,_3,_4,_5,_6,_7, NAME, ...) NAME

#define Entity(...) GET_MACRO_ENTITY(__VA_ARGS__, ENTITY7, ENTITY6 ,ENTITY5, ENTITY4, ENTITY3, ENTITY2, ENTITY1)(__VA_ARGS__)

#define Trait(Name) Name : T_EntityTraitBase<Name>

#ifndef WITH_EDITOR
	#define Reflected()
	#define Field(Type, Name, ...) __VA_ARGS__ Type Name
	#define Name(Type, Name, ...) __VA_ARGS__ Type Name 
#endif


/** Global entity type system data */
namespace EntityTypeSystem
{
	static inline u32 TypeIDCounter = 0;
}

/** Used to auto register a new entity type */
template<typename T_Entity>
struct T_EntityTypeBase
{
	static inline Array<TraitID, EntityTraitsManager::max_traits> traits{};
	// max instances per world chunk
	static inline u32 instance_budget = 10;
private:
	static inline TypeID TYPE_ID;

public:
	// sets the type_id at construction
	T_EntityTypeBase()
	{
		// TODO: See if we really need the static helper here. (why not use directly TYPE_ID?)
		STATIC_HELPER int type_id = ++EntityTypeSystem::TypeIDCounter;
		TYPE_ID = type_id;
		reinterpret_cast<E_BaseEntity*>(this)->type_id = TYPE_ID;
	};

	static TypeID GetTypeId() { return TYPE_ID; };
};

/** Used to auto register a new trait type */
template<typename T_Trait>
struct T_EntityTraitBase
{
	static inline TraitID trait_id;

	// ReSharper disable once CppFunctionIsNotImplemented
	template<typename T_Entity>
	static void Update(T_Entity& entity);

	T_EntityTraitBase() 
	{
		// TODO: Same question here, is it necessary to have a static helper? I am almost sure yes but don't understand why right now.
		STATIC_HELPER TraitID get_id = []()
		{  
			auto* etm = EntityTraitsManager::Get();
			// TODO: Doing it like that, TraitIDs are not persistent. Do we need them to be?
			TraitID trait_id = etm->entity_traits.size();
			etm->entity_traits.push_back(trait_id);
			return trait_id;
		}();
		
		trait_id = get_id;
	};

};

/** Used to auto match an entity type and a trait type */
template<typename T_Entity, typename T_Trait>
struct T_TraitMixin : T_Trait
{
	StaticHelperByte helper_byte  = EntityTraitsManager::Get()->RegisterTypeAndTraitMatch<T_Entity, T_Trait>();
	// force templated static member to be initialized
	static_assert(&helper_byte);
};
