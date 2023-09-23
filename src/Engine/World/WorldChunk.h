#pragma once

#include "WorldChunkStorage.h"
#include "engine/core/core.h"

// how many cells we have preallocated for the world
constexpr int WorldChunkNumX = 1;
constexpr int WorldChunkNumY = 1;
constexpr int WorldChunkNumZ = 1;

// how many cells are before and after the origin in each axis
constexpr float WorldChunkOffsetX = WorldChunkNumX / 2.f;
constexpr float WorldChunkOffsetY = WorldChunkNumY / 2.f;
constexpr float WorldChunkOffsetZ = WorldChunkNumZ / 2.f;

// how many meters the cell occupies in the world
constexpr float WorldChunkLengthMeters = 5000.0f;

constexpr vec3 WUpperBoundsMeters = {
	WorldChunkOffsetX * WorldChunkLengthMeters,
	WorldChunkOffsetY * WorldChunkLengthMeters,
	WorldChunkOffsetZ * WorldChunkLengthMeters
	};

constexpr vec3 WLowerBoundsMeters = {
	-1.0 * WorldChunkOffsetX * WorldChunkLengthMeters,
	-1.0 * WorldChunkOffsetX * WorldChunkLengthMeters,
	-1.0 * WorldChunkOffsetX * WorldChunkLengthMeters
	};


vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k);
struct WorldChunkPosition WorldCoordsToCells(float x, float y, float z);
struct WorldChunkPosition WorldCoordsToCells(vec3 position);

struct WorldChunkPosition
{
	i32 i = -1;
	i32 j = -1;
	i32 k = -1;

	WorldChunkPosition() = default;
	WorldChunkPosition(int i, int j, int k) : i(i), j(j), k(k){}

	vec3 GetVec() { return vec3(i,j,k); }
	
	bool operator == (const WorldChunkPosition& other) { return other.i == i && other.j == j && other.k == k;}
	bool operator == (const vec3& other) { return other.x == i && other.y == j && other.z == k; }
	bool operator < (const WorldChunkPosition& other)  const { return other.i < i && other.j < j && other.k < k; }
};

struct WorldChunk
{
	using byte = char;

	friend struct WorldChunkEntityIterator;
	
	// TODO: World chunk ID needs to come from it's world position
	static inline u32 world_chunk_id_count = 0;
	
	u32 id = ++world_chunk_id_count;

	// indexes
	int i = -1;
	int j = -1;
	int k = -1;

	// Low level byte array
	ChunkStorage chunk_storage;

	//TODO: I don't think we need this
	vector<E_Entity*> visitors{};

	WorldChunk() : chunk_storage(id) {} 
	WorldChunk(u32 i, u32 j, u32 k) : i(i), j(j), k(k), chunk_storage(id) {}
	
	WorldChunkEntityIterator GetIterator();

	WorldChunkPosition GetPosition() { return WorldChunkPosition(i, j, k); }

	void RemoveEntity(E_Entity* entity);
	bool AddVisitor(E_Entity* entity);
	bool RemoveVisitor(E_Entity* entity);

	template<typename T_Entity>
	T_Entity* AddEntity()
	{
		return chunk_storage.RequestEntityStorage<T_Entity>();
	}
	
	vec3 GetPositionMetric();
	WorldChunkPosition GetChunkPosition() { return WorldChunkPosition(i,j,k); }
	string GetChunkPositionString();
	string GetChunkPositionMetricString();
	
	void InvokeTraitUpdateOnAllTypes(TraitID trait_id);
};