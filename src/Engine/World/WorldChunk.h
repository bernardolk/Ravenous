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

constexpr vec3 WUpperBoundsMeters =
{
	WorldChunkOffsetX * WorldChunkLengthMeters,
	WorldChunkOffsetY * WorldChunkLengthMeters,
	WorldChunkOffsetZ * WorldChunkLengthMeters
};

constexpr vec3 WLowerBoundsMeters =
{
-1.0 * WorldChunkOffsetX * WorldChunkLengthMeters,
-1.0 * WorldChunkOffsetX * WorldChunkLengthMeters,
-1.0 * WorldChunkOffsetX * WorldChunkLengthMeters
};


vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k);
struct RWorldChunkPosition WorldCoordsToCells(float X, float Y, float Z);
struct RWorldChunkPosition WorldCoordsToCells(vec3 Position);

struct RWorldChunkPosition
{
	int i = -1;
	int j = -1;
	int k = -1;

	RWorldChunkPosition() = default;
	RWorldChunkPosition(int I, int J, int K) :
		i(I), j(J), k(K) {}

	vec3 GetVec() { return vec3(i, j, k); }

	bool operator ==(const RWorldChunkPosition& Other) { return Other.i == i && Other.j == j && Other.k == k; }
	bool operator ==(const vec3& Other) { return Other.x == i && Other.y == j && Other.z == k; }
	bool operator <(const RWorldChunkPosition& Other) const { return Other.i < i && Other.j < j && Other.k < k; }
};

struct RWorldChunk
{
	using byte = char;

	friend struct RWorldChunkEntityIterator;

	// TODO: World chunk ID needs to come from it's world position
	static inline uint WorldChunkIdCount = 0;

	uint id = ++WorldChunkIdCount;

	// indexes
	int i = -1;
	int j = -1;
	int k = -1;

	// Low level byte array
	RWorldChunkStorage ChunkStorage;

	//TODO: I don't think we need this
	vector<EEntity*> Visitors;

	RWorldChunk() : ChunkStorage(id) {}
	RWorldChunk(uint I, uint J, uint K) : i(I), j(J), k(K), ChunkStorage(id) {}

	RWorldChunkEntityIterator GetIterator();

	RWorldChunkPosition GetPosition() { return RWorldChunkPosition(i, j, k); }

	void RemoveEntity(EEntity* Entity);
	bool AddVisitor(EEntity* Entity);
	bool RemoveVisitor(EEntity* Entity);

	template<typename TEntity>
	TEntity* AddEntity()
	{
		return ChunkStorage.RequestEntityStorage<TEntity>();
	}

	vec3 GetPositionMetric();
	RWorldChunkPosition GetChunkPosition() { return RWorldChunkPosition(i, j, k); }
	string GetChunkPositionString();
	string GetChunkPositionMetricString();

	void InvokeTraitUpdateOnAllTypes(TraitID TraitId);
};
