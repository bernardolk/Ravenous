#pragma once

#include "engine/core/core.h"
#include "engine/entities/Entity.h"
#include "Engine/Entities/Traits/EntityTraitsManager.h"

// Define empty macros for editor-only features in shipping build.
#ifndef WITH_EDITOR
	#define Reflected()
	#define Field(Type, Name, ...) __VA_ARGS__ Type Name
	#define Name(Type, Name, ...) __VA_ARGS__ Type Name 
#endif

// We forward declare the type, then store the typename in a helper. Only after we actually define the type.
// Reflection is only necessary in editor builds. We shouldn't ship the game with reflection on unless really necessary.
#ifdef WITH_EDITOR
	#include "editor/reflection/Reflection.h"
	#define ENTITY1(TypeName)                               						TypeName; STORE_TYPE_IN_HELPER(TypeName); struct TypeName : EEntity, T_EntityTypeBase<TypeName>, Reflection::ReflectionGetterSetter<TypeName>
	#define ENTITY2(TypeName, Trait1)                       						TypeName; STORE_TYPE_IN_HELPER(TypeName); struct TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, Reflection::ReflectionGetterSetter<TypeName, Trait1>
	#define ENTITY3(TypeName, Trait1, Trait2)               						TypeName; STORE_TYPE_IN_HELPER(TypeName); struct TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2>
	#define ENTITY4(TypeName, Trait1, Trait2, Trait3)								TypeName; STORE_TYPE_IN_HELPER(TypeName); struct TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2, Trait3>
	#define ENTITY5(TypeName, Trait1, Trait2, Trait3, Trait4)						TypeName; STORE_TYPE_IN_HELPER(TypeName); struct TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2, Trait3, Trait4>
	#define ENTITY6(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5)				TypeName; STORE_TYPE_IN_HELPER(TypeName); struct TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, T_TraitMixin<TypeName, Trait5>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2, Trait3, Trait4, Trait5>
	#define ENTITY7(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5, Trait6)       TypeName; STORE_TYPE_IN_HELPER(TypeName); struct TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, T_TraitMixin<TypeName, Trait5>, T_TraitMixin<TypeName, Trait6>, Reflection::ReflectionGetterSetter<TypeName, Trait1, Trait2, Trait3, Trait4, Trait5, Trait6>
#else
	#define ENTITY1(TypeName)                               						TypeName : EEntity, T_EntityTypeBase<TypeName>
	#define ENTITY2(TypeName, Trait1)                       						TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>
	#define ENTITY3(TypeName, Trait1, Trait2)               						TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>
	#define ENTITY4(TypeName, Trait1, Trait2, Trait3)       						TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>
	#define ENTITY5(TypeName, Trait1, Trait2, Trait3, Trait4)						TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>
	#define ENTITY6(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5)				TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, T_TraitMixin<TypeName, Trait5>
	#define ENTITY7(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5, Trait6)       TypeName : EEntity, T_EntityTypeBase<TypeName>, T_TraitMixin<TypeName, Trait1>, T_TraitMixin<TypeName, Trait2>, T_TraitMixin<TypeName, Trait3>, T_TraitMixin<TypeName, Trait4>, T_TraitMixin<TypeName, Trait5>, T_TraitMixin<TypeName, Trait6>
#endif

#define GET_MACRO_ENTITY(_1,_2,_3,_4,_5,_6,_7, NAME, ...) NAME

#define Entity(...) GET_MACRO_ENTITY(__VA_ARGS__, ENTITY7, ENTITY6 ,ENTITY5, ENTITY4, ENTITY3, ENTITY2, ENTITY1)(__VA_ARGS__)

#define Trait(Name) Name : T_EntityTraitBase<Name>


/** Global entity type system data */
namespace EntityTypeSystem
{
	static inline uint TypeIDCounter = 0;
}

/** Used to auto register a new entity type */
template<typename T_Entity>
struct T_EntityTypeBase
{
	static inline Array<TraitID, EntityTraitsManager::max_traits> traits;
	// max instances per world chunk
	static inline uint instance_budget = 10;
private:
	static inline TypeID TYPE_ID = ++EntityTypeSystem::TypeIDCounter;

public:
	// sets the type_id at construction
	T_EntityTypeBase()
	{
		reinterpret_cast<EEntity*>(this)->type_id = TYPE_ID;
	};

	static TypeID GetTypeId() { return TYPE_ID; }
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
		StaticHelper TraitID get_id = []
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
