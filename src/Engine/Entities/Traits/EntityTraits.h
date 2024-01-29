#pragma once

#include "Engine/Entities/Traits/EntityTraitsManager.h"

#define WITH_EDITOR 1

// We forward declare the type, then store the typename in a helper. Only after we actually define the type.
// Reflection is only necessary in editor builds. We shouldn't ship the game with reflection on unless really necessary.
#ifdef WITH_EDITOR
	#include "editor/reflection/Reflection.h"
	#define BaseEntityType(TypeName)												TypeName : Reflection::ReflectionGetterSetter<TypeName>, Reflection::Tracer
	#define ENTITY1(TypeName)                               						TypeName : EEntity, TEntityTypeBase<TypeName>, Reflection::ReflectionGetterSetter<TypeName, EEntity>, Reflection::Tracer
	#define ENTITY2(TypeName, Trait1)                       						TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, Reflection::ReflectionGetterSetter<TypeName, EEntity, Trait1>, Reflection::Tracer
	#define ENTITY3(TypeName, Trait1, Trait2)               						TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>, Reflection::ReflectionGetterSetter<TypeName, EEntity, Trait1, Trait2>, Reflection::Tracer
	#define ENTITY4(TypeName, Trait1, Trait2, Trait3)								TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>, TTraitMixin<TypeName, Trait3>, Reflection::ReflectionGetterSetter<TypeName, EEntity, Trait1, Trait2, Trait3>, Reflection::Tracer
	#define ENTITY5(TypeName, Trait1, Trait2, Trait3, Trait4)						TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>, TTraitMixin<TypeName, Trait3>, TTraitMixin<TypeName, Trait4>, Reflection::ReflectionGetterSetter<TypeName, EEntity, Trait1, Trait2, Trait3, Trait4>, Reflection::Tracer
	#define ENTITY6(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5)				TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>, TTraitMixin<TypeName, Trait3>, TTraitMixin<TypeName, Trait4>, TTraitMixin<TypeName, Trait5>, Reflection::ReflectionGetterSetter<TypeName, EEntity, Trait1, Trait2, Trait3, Trait4, Trait5>, Reflection::Tracer
	#define ENTITY7(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5, Trait6)       TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>, TTraitMixin<TypeName, Trait3>, TTraitMixin<TypeName, Trait4>, TTraitMixin<TypeName, Trait5>, TTraitMixin<TypeName, Trait6>, Reflection::ReflectionGetterSetter<TypeName, EEntity, Trait1, Trait2, Trait3, Trait4, Trait5, Trait6>, Reflection::Tracer
#else
	#define BaseEntityType(TypeName)												TypeName
	#define ENTITY1(TypeName)                               						TypeName : EEntity, TEntityTypeBase<TypeName>
	#define ENTITY2(TypeName, Trait1)                       						TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>
	#define ENTITY3(TypeName, Trait1, Trait2)               						TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>
	#define ENTITY4(TypeName, Trait1, Trait2, Trait3)       						TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>, TTraitMixin<TypeName, Trait3>
	#define ENTITY5(TypeName, Trait1, Trait2, Trait3, Trait4)						TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>, TTraitMixin<TypeName, Trait3>, TTraitMixin<TypeName, Trait4>
	#define ENTITY6(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5)				TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>, TTraitMixin<TypeName, Trait3>, TTraitMixin<TypeName, Trait4>, TTraitMixin<TypeName, Trait5>
	#define ENTITY7(TypeName, Trait1, Trait2, Trait3, Trait4, Trait5, Trait6)       TypeName : EEntity, TEntityTypeBase<TypeName>, TTraitMixin<TypeName, Trait1>, TTraitMixin<TypeName, Trait2>, TTraitMixin<TypeName, Trait3>, TTraitMixin<TypeName, Trait4>, TTraitMixin<TypeName, Trait5>, TTraitMixin<TypeName, Trait6>
	#define Reflected()
	#define Field(Type, Name, ...) __VA_ARGS__ Type Name
	#define Name(Type, Name, ...) __VA_ARGS__ Type Name
#endif

#define SELECT_MACRO_ENTITY_FROM_VARGS(_1,_2,_3,_4,_5,_6,_7, NAME, ...) NAME

#define EntityType(...) SELECT_MACRO_ENTITY_FROM_VARGS(__VA_ARGS__, ENTITY7, ENTITY6 ,ENTITY5, ENTITY4, ENTITY3, ENTITY2, ENTITY1)(__VA_ARGS__)

#define Trait(Name) Name : TEntityTraitBase<Name>

template<typename T> constexpr bool DependentFalse = false;

#define REQUIRES_METHOD(x)

/* ===================================================================
 * EntityTypeSystem
 *	Global entity type system data
 * =================================================================== */
namespace EntityTypeSystem
{
	inline uint GetNextTypeID()
	{
		static uint TypeIDCounter = 0;
		return ++TypeIDCounter;
	}
}

/* ===================================================================
 * TEntityTypeBase (Template Base Class):
 *	Used to provide basic Entity type-specific data 
 * =================================================================== */
// We store TypeID in each instance on construction so that we can decide where to store in memory each entity based on its TypeID. Basically, if
// we have an EEntity* we can look the TypeID and perform a "cast" to the correct underlying entity type. We can't use the static RTypeID because
// that requires type information to be resolved whereas instance based TypeID is always available for any EEntity.
template<typename TEntity>
struct TEntityTypeBase
{
	static REntityTypeID GetTypeID()
	{
		static REntityTypeID EntityTypeID = EntityTypeSystem::GetNextTypeID();
		return EntityTypeID;
	}
	
	// Default instance budget for any EEntity subtype. Can be overriden by each type.
	static inline uint InstanceBudget = 10;

	/* =================
	 *	Entity Traits
	 * ================= */	
	static inline Array<RTraitID, EntityTraitsManager::MaxTraits> Traits;

protected:
	TEntityTypeBase()
	{
		// Sets TypeID in the instance being constructed
		static_cast<TEntity*>(this)->TypeID = GetTypeID();
	};
};

/* ===================================================================
 * TEntityTraitBase (Template Base Class):
 *	Used to provide basic Trait type-specific data 
 * =================================================================== */
template<typename TTrait>
struct TEntityTraitBase
{
	static inline RTraitID TraitID;

	template<typename TEntity>
	static void Update(TEntity& Entity)
	{
		static_assert(DependentFalse<TEntity>, "You forgot to implement Update function for a Trait");
	};

	TEntityTraitBase()
	{
		static RTraitID TTraitID = [] {
			auto* Etm = EntityTraitsManager::Get();
			// TODO: Make TraitIDs persistent, not based on initialization order of instances (maybe)
			RTraitID TraitId = Etm->TraitsRegistry.size();
			Etm->TraitsRegistry.push_back(TraitId);
			return TraitId;
		}();

		TraitID = TTraitID;
	}

};

/* ===================================================================
 * TTraitMixin (Template Base Class):
 *	Used to auto match an entity type and a trait type 
 * =================================================================== */
template<typename TEntity, typename TTrait>
struct TTraitMixin : TTrait
{
	static inline char HelperByte = (EntityTraitsManager::Get()->RegisterTypeAndTraitMatch<TEntity, TTrait>(), 0);
	static_assert(&HelperByte);
};
