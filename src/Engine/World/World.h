#pragma once

#include "WorldChunk.h"
#include "engine/collision/raycast.h"
#include "engine/core/core.h"
#include "engine/entities/Entity.h"
#include "engine/utils/utils.h"

namespace RavenousEngine
{
	struct RFrameData;
}
struct EEntity;
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
	DeclSingleton(RWorld)
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

	template<typename TEntity>
	TEntity* SpawnEntity();

	template<typename TEntity>
	TEntity* RWorld::SpawnEntityAtPosition(vec3 Position);

	TIterator<RWorldChunk> GetChunkIterator();
	static WorldEntityIterator GetEntityIterator();

	RRaycastTest Raycast(RRay Ray, NRayCastType TestType, const EEntity* Skip = nullptr, float MaxDistance = MaxFloat) const;
	RRaycastTest Raycast(RRay Ray, const EEntity* Skip = nullptr, float MaxDistance = MaxFloat) const;
	RRaycastTest LinearRaycastArray(RRay FirstRay, int Qty, float Spacing) const;
	RRaycastTest RaycastLights(RRay Ray) const;

	CellUpdate UpdateEntityWorldChunk(EEntity* Entity);

	RavenousEngine::RFrameData& GetFrameData();

private:
	void UpdateTraits();
	void UpdateTransforms();
};

struct WorldEntityIterator
{
	uint8 TotalActiveChunks = 0;
	uint8 CurrentChunkIndex = 0;

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
	auto* FirstChunkAvailable = ChunksMap.begin()->second;
	if (!FirstChunkAvailable)
		return nullptr;

	return FirstChunkAvailable->AddEntity<TEntity>();
}

template<typename TEntity>
TEntity* RWorld::SpawnEntityAtPosition(vec3 Position)
{
	auto* FirstChunkAvailable = ChunksMap.begin()->second;
	if (!FirstChunkAvailable)
		return nullptr;

	auto* Entity = FirstChunkAvailable->RequestEntityStorage<TEntity>();
	if (Entity)
	{
		Entity->position = Position;
		UpdateEntityWorldChunk(Entity);
	}

	return Entity;
}
