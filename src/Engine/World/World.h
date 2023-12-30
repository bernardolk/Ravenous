#pragma once

#include "WorldChunk.h"
#include "engine/collision/raycast.h"
#include "engine/core/core.h"
#include "Engine/Core/UUIDGenerator.h"

namespace RavenousEngine
{
	struct RFrameData;
}

struct RWorldChunkPosition WorldCoordsToCells(float X, float Y, float Z);
vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k);

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
	friend WorldEntityIterator;
	
	static RWorld* Get()
	{
		static RWorld Instance{};
		return &Instance;
	}
	
	// static constexpr u8 world_chunk_matrix_order = 10;
	static constexpr uint WorldSizeInChunks = WorldChunkNumX * WorldChunkNumY * WorldChunkNumZ;

	string SceneName;

	// TODO: We can't use world chunk "matrix" position as its ijk position! This is insane! What if we want to unload part A of the world and load part B,
	//		what are the index going to say? Nothing.
	//		in the future such vector will be replaced with a memory arena
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

	void Update();
	void Erase();

	template<typename TEntity>
	TEntity* SpawnEntity();

	template<typename TEntity>
	TEntity* SpawnEntityAtPosition(vec3 Position);

	TIterator<RWorldChunk> GetChunkIterator();
	static WorldEntityIterator GetEntityIterator();

	RRaycastTest Raycast(RRay Ray, NRayCastType TestType, const EEntity* Skip = nullptr, float MaxDistance = MaxFloat) const;
	RRaycastTest Raycast(RRay Ray, const EEntity* Skip = nullptr, float MaxDistance = MaxFloat) const;
	RRaycastTest LinearRaycastArray(RRay FirstRay, int Qty, float Spacing) const;
	RRaycastTest RaycastLights(RRay Ray) const;

	CellUpdate UpdateEntityWorldChunk(EEntity* Entity);

	RavenousEngine::RFrameData& GetFrameData();


private:
	RWorld();

	// This was introduced so that we can go back to the saner vector list of entities approach.
	// Cranking out space partitioning and generic entity storage solutions was premature optimization and it severely hurts debugging since entities are reduced to bytes in a memory arena.
	vector<EEntity*> EntityList{};
	
	void UpdateTraits();
	void UpdateTransforms();
};

struct WorldEntityIterator
{
	uint8 TotalActiveChunks = 0;
	uint8 CurrentChunkIndex = 0;

	// This was introduced so that we can go back to the saner vector list of entities approach.
	// Cranking out space partitioning and generic entity storage solutions was premature optimization and it severely hurts debugging since entities are reduced to bytes in a memory arena.
	uint EntityVectorIndex = 0;

	RWorld* World;
	RWorldChunkEntityIterator ChunkIterator;

	WorldEntityIterator();

	EEntity* operator()();
};


//TODO: Move these somewhere else
void SetEntityDefaultAssets(EEntity* Entity);
void SetEntityAssets(EEntity* Entity, struct REntityAttributes Attrs);


template<typename TEntity>
TEntity* RWorld::SpawnEntity()
{
	// This was introduced so that we can go back to the saner vector list of entities approach.
	// Cranking out space partitioning and generic entity storage solutions was premature optimization and it severely hurts debugging since entities are reduced to bytes in a memory arena.
	if (auto* NewEntity = new TEntity)
	{
		NewEntity->ID = RUUIDGenerator::GetNewRUUID();
		EntityList.push_back(NewEntity);
		return NewEntity;
	}
	
	return nullptr;
	
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
inline EEntity* RWorld::SpawnEntity<EEntity>()
{
	return nullptr;
}


template<typename TEntity>
TEntity* RWorld::SpawnEntityAtPosition(vec3 Position)
{
	auto* FirstChunkAvailable = ChunksMap.begin()->second;
	if (!FirstChunkAvailable)
		return nullptr;

	auto* Entity = FirstChunkAvailable->AddEntity<TEntity>();
	if (Entity)
	{
		Entity->position = Position;
		UpdateEntityWorldChunk(Entity);
	}

	return Entity;
}
