#pragma once

#include "EntitySlot.h"
#include "WorldChunk.h"
#include "engine/collision/raycast.h"
#include "engine/core/core.h"
#include "Engine/Core/UUIDGenerator.h"
#include "Engine/Entities/EHandle.h"

namespace RavenousEngine
{
	struct RFrameData;
}

enum CellUpdateStatus
{
	CellUpdate_OK,
	CellUpdate_ENTITY_TOO_BIG,
	CellUpdate_CELL_FULL,
	CellUpdate_OUT_OF_BOUNDS,
	CellUpdate_UNEXPECTED
};

struct CellUpdate
{
	CellUpdateStatus Status = CellUpdate_UNEXPECTED;
	string Message;
	bool EntityChangedCell = false;
};


struct RWorld
{
	friend REntityIterator;
	template<typename T> friend EHandle<T> MakeHandle(EEntity* Entity);
	template<typename TEntity> friend EHandle<TEntity> SpawnEntity();
	template<typename TEntity> friend void DeleteEntity(EHandle<TEntity> Entity);
	template<typename TEntity> friend EHandle<TEntity> MakeHandleFromID(RUUID ID);
	
	static RWorld* Get()
	{
		static RWorld Instance{};
		return &Instance;
	}

	// ====================
	//  DATA
	// ====================
	// static constexpr u8 world_chunk_matrix_order = 10;
	static constexpr uint WorldSizeInChunks = WorldChunkNumX * WorldChunkNumY * WorldChunkNumZ;

	// TODO: deleteme
	string SceneName;

	// TODO: We can't use world chunk "matrix" position as its ijk position! This is insane! What if we want to unload part A of the world and load part B, what are the index going to say? Nothing in the future such vector will be replaced with a memory arena
	Array<RWorldChunk, WorldSizeInChunks> Chunks;
	map<RWorldChunkPosition, RWorldChunk*> ChunksMap;
	vector<RWorldChunk*> ActiveChunks;

	float GlobalShininess = 17;
	float AmbientIntensity = 0;
	vec3 AmbientLight = vec3(1);

	// temp
	vector<EPointLight*> PointLights;
	vector<ESpotLight*> SpotLights;
	vector<EDirectionalLight*> DirectionalLights;
	// end temp

	// ====================
	//	METHODS
	// ====================
public:
	void Update();
	void Erase();
	void DeleteEntitiesMarkedForDeletion();

	TIterator<RWorldChunk> GetChunkIterator();

	RRaycastTest Raycast(const RRay& Ray, NRayCastType TestType, const EEntity* Skip = nullptr, float MaxDistance = MaxFloat) const;
	RRaycastTest Raycast(const RRay& Ray, const EEntity* Skip = nullptr, float MaxDistance = MaxFloat) const;
	RRaycastTest LinearRaycastArray(RRay FirstRay, int Qty, float Spacing) const;
	RRaycastTest RaycastLights(RRay Ray) const;

	CellUpdate UpdateEntityWorldChunk(EEntity* Entity);

	RavenousEngine::RFrameData& GetFrameData();

	[[nodiscard]] bool IsEntitySlotValid(const REntitySlot& Slot) const;

	void UpdateTraits();
	void UpdateTransforms();
	
private:
	RWorld();

	REntityStorage EntityStorage;
	vector<RView<REntitySlot>> EntitiesToDelete;
};

struct REntityIterator
{
	// uint8 TotalActiveChunks = 0;
	// uint8 CurrentChunkIndex = 0;
	// RWorldChunkEntityIterator ChunkIterator;

	RWorld* World;
	REntitySlot* CurrentEntitySlot = nullptr;

	REntityIterator();
	EEntity* operator()();
};


//TODO: Move these somewhere else
void SetEntityDefaultAssets(EEntity* Entity);
void SetEntityAssets(EEntity* Entity, struct REntityAttributes Attrs);


template<typename TEntity>
EHandle<TEntity> SpawnEntity()
{
	// This was introduced so that we can go back to the saner vector list of entities approach.
	// Cranking out space partitioning and generic entity storage solutions was premature optimization and it severely hurts debugging since entities are reduced to bytes in a memory arena.
	if (auto* NewEntity = new TEntity)
	{
		auto* World = RWorld::Get();
		NewEntity->ID = RUUIDGenerator::GetNewRUUID();
		auto* Slot = World->EntityStorage.Add(NewEntity);
		return EHandle<TEntity>{World->EntityStorage.EntitySlots, *Slot, Slot->Generation};
	}
	
	return {};
	
#if 0
	auto* FirstChunkAvailable = ChunksMap.begin()->second;
	if (!FirstChunkAvailable)
		return nullptr;

	TEntity* NewEntity = FirstChunkAvailable->AddEntity<TEntity>();
	NewEntity->ID = RUUIDGenerator::GetNewRUUID();
	
	return NewEntity;
#endif
}

/* =======================================
/* EEntity Specialization
/* ======================================= */
// EEntity is an abstract type, it does not contain budget or traits info and shouldn't
// be directly spanwed. EStaticMesh is the correct basic entity type.
template<>
inline EHandle<EEntity> SpawnEntity<EEntity>()
{
	return {};
}

template<typename TEntity>
EHandle<TEntity> MakeHandle(EEntity* Entity)
{
	auto* World = RWorld::Get();
	for (auto& Slot : World->EntityStorage.EntitySlots) {
		if (Slot.Value == Entity) {
			return EHandle<TEntity>{World->EntityStorage.EntitySlots, Slot, Slot.Generation};
		}
	}

	// If this this hits, this means there is a dangling entity ptr somewhere. The entity was freed but this ptr still references it. Investigate!
	assert(false);
	return {};
}

template<typename TEntity>
EHandle<TEntity> MakeHandleFromID(RUUID ID)
{
	auto* World = RWorld::Get();
	for (auto& Slot : World->EntityStorage.EntitySlots)
	{
		bool bSkip = false;
		for (auto* EmptySlot : World->EntityStorage.EmptySlots) {
			if (&Slot == EmptySlot) {
				bSkip = true;
			}
		}
		if (bSkip)
			continue;

		// Todo: I don't like how much work this is. This seems very expensive.
		if (GetID(Slot) == ID) {
			return {World->EntityStorage.EntitySlots, Slot, Slot.Generation};
		}
	}

	return {};
}

// The rationale behind why this takes a handle is twofold: First, it is more performant to figure out where the entity is if we have a reference to the entity slot (just take the offset in the vector)
// Second, it suggests to callers to make handles and store these instead of raw entity ptrs.
template<typename TEntity>
void DeleteEntity(EHandle<TEntity> Entity)
{
	if (!Entity.IsValid()) return;
	RWorld::Get()->EntitiesToDelete.push_back(Entity.Slot);
}