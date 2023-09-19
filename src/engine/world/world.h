#pragma once

#include "engine/collision/raycast.h"
#include "engine/core/core.h"
#include "engine/entities/e_entity.h"
#include "engine/entities/e_entity.h"
#include "engine/entities/manager/entity_traits_manager.h"
#include "engine/utils/utils.h"

struct E_Entity;
struct WorldChunkPosition WorldCoordsToCells(float x, float y, float z);
vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k);

// how many cells we have preallocated for the world
constexpr static int WorldChunkNumX = 1;
constexpr static int WorldChunkNumY = 1;
constexpr static int WorldChunkNumZ = 1;

// how many cells are before and after the origin in each axis
constexpr static float WorldChunkOffsetX = WorldChunkNumX / 2.f;
constexpr static float WorldChunkOffsetY = WorldChunkNumY / 2.f;
constexpr static float WorldChunkOffsetZ = WorldChunkNumZ / 2.f;

// how many meters the cell occupies in the world
constexpr static float WorldChunkLengthMeters = 5000.0f;

const static vec3 WUpperBoundsMeters = {
	WorldChunkOffsetX * WorldChunkLengthMeters,
	WorldChunkOffsetY * WorldChunkLengthMeters,
	WorldChunkOffsetZ * WorldChunkLengthMeters
	};

const static vec3 WLowerBoundsMeters = {
	-1.0 * WorldChunkOffsetX * WorldChunkLengthMeters,
	-1.0 * WorldChunkOffsetX * WorldChunkLengthMeters,
	-1.0 * WorldChunkOffsetX * WorldChunkLengthMeters
	};

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
	CellUpdateStatus status{};
	std::string message{};
	bool entity_changed_cell = false;
};

/**
 * TODO:
 * . Add exclude / delete entity functionality
 * . 
 */

/**
 *  World / World Chunk memory layout brief explanation:
 *  The world is composed of chunks, each chunk has a memory budget in bytes and can store blocks of entities.
 *  Each block can contain only one entity type. Each block vary in size depending on each entity type's memory budget.
 *  Each block is also consecutive in memory inside a world chunk's storage.
 */

/** Structure that maintains metadata about entity storage. Does not actually store any data. */
struct EntityStorageBlockMetadata
{
	using byte = char;
	
	// type data
	TypeID type_id;
	u32 type_size;
	// TODO: We don't need to know about traits at this level, lets only deal with entity Types
	Array<TraitID, EntityTraitsManager::max_traits> entity_traits{};

	// array bookkeeping
	u32 max_entity_instances;
	int entity_count = 0;
	byte* data_start = nullptr;
};

/** Memory Arena for storing all the world chunk's entities's data. */
struct ChunkStorage
{
	using byte = char;

	// MAX BUDGET for ALL entities
	inline static constexpr u32 chunk_byte_budget = 666000;
	
	byte data[chunk_byte_budget]{};
	byte* next_block_start = &data[0];
	u32 bytes_consumed = 0;
};

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
	static inline constexpr u32 max_types_per_storage = 20;
	
    u32 id = ++world_chunk_id_count;

	// indexes
	int i = -1;
	int j = -1;
	int k = -1;

	// Low level byte array
	ChunkStorage chunk_storage;
	// Holds the useful metadata about each storage block in chunk storage. Iterate over it to go through all entity types.
	Array<EntityStorageBlockMetadata, max_types_per_storage> storage_metadata_array;
	// Convenience map to retrieve storage metadata for a specific entity type.
    map<TypeID, EntityStorageBlockMetadata*> storage_metadata_map;
	// TODO: Implement a TraitID to array of typeIds

	vector<E_Entity*> visitors{};

	WorldChunk() = default;
	WorldChunk(u32 i, u32 j, u32 k) : i(i), j(j), k(k) {}
	
	WorldChunkEntityIterator GetIterator();

	WorldChunkPosition GetPosition() { return WorldChunkPosition(i, j, k); }

	void RemoveEntity(E_Entity* entity);

	bool AddVisitor(E_Entity* entity);
	bool RemoveVisitor(E_Entity* entity);

	vec3 GetPositionMetric();
	
	WorldChunkPosition GetChunkPosition() { return WorldChunkPosition(i,j,k); }

	string GetChunkPositionString();

	string GetChunkPositionMetricString();
	
    void InvokeTraitUpdateOnAllTypes(TraitID trait_id);

	template<typename T_Entity>
	void WorldChunk::MaybeAllocateForType();

	template<typename T_Entity>
	T_Entity* WorldChunk::RequestEntityStorage();
};


struct WorldChunkEntityIterator
{
	WorldChunk* chunk;
	u32 block_idx = 0;
	u32 entity_idx = 0;
	
	explicit WorldChunkEntityIterator(WorldChunk* chunk) : chunk(chunk){};

	E_Entity* operator()();
};


/** World */
struct WorldEntityIterator;

struct T_World
{
	// static constexpr u8 world_chunk_matrix_order = 10;
	static constexpr u32 world_size_in_chunks = WorldChunkNumX * WorldChunkNumY * WorldChunkNumZ;

	string scene_name;

	// TODO: We can't use world chunk "matrix" position as its ijk position! This is insane! What if we want to unload part A of the world and load part B,
	//		what are the index going to say? Nothing.
	//		in the future such vector will be replaced with a memory arena
	Array<WorldChunk, world_size_in_chunks> chunks;
	map<WorldChunkPosition, WorldChunk*> chunks_map;
	vector<WorldChunk*> active_chunks;

	float global_shininess = 17;
	float ambient_intensity = 0;
	vec3 ambient_light = vec3(1);

	// temp
	vector<PointLight*> point_lights;
	vector<SpotLight*> spot_lights;
	vector<DirectionalLight*> directional_lights;
	// end temp
		
	void Update();

	static T_World* Get()
	{
		static T_World instance;
		return &instance;
	}

	template<typename T_Entity>
	T_Entity* CreateEntity(u8 i, u8 j, u8 k);

	Iterator<WorldChunk> GetChunkIterator();
	static WorldEntityIterator GetEntityIterator();	
	
	RaycastTest Raycast(Ray ray, RayCastType test_type, const E_Entity* skip = nullptr, float max_distance = MaxFloat) const;
	RaycastTest Raycast(Ray ray, const E_Entity* skip = nullptr, float max_distance = MaxFloat) const;
	RaycastTest LinearRaycastArray(Ray first_ray, int qty, float spacing) const;
	RaycastTest RaycastLights(Ray ray) const;

	CellUpdate UpdateEntityWorldCells(E_Entity* entity);
	
private:
	T_World();

	void UpdateTraits();
	void UpdateTransforms();
};

struct WorldEntityIterator
{
	u8 total_active_chunks = 0;
	u8 current_chunk_index = 0;

	T_World* world;
	WorldChunkEntityIterator chunk_iterator;
	
	WorldEntityIterator();
	
	E_Entity* operator()();
};

vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k);
WorldChunkPosition WorldCoordsToCells(float x, float y, float z);
WorldChunkPosition WorldCoordsToCells(vec3 position);


//TODO: Move these somewhere else
void SetEntityDefaultAssets(E_Entity* entity);
void SetEntityAssets(E_Entity* entity, struct EntityAttributes attrs);



template<typename T_Entity>
void WorldChunk::MaybeAllocateForType()
{
	auto type_id = T_Entity::GetTypeId();

	if (storage_metadata_map.contains(type_id))
		return;

	// if we have budget, get a block for the new entity type
	u32 total_entity_block_size = T_Entity::instance_budget * sizeof(T_Entity);
	if(chunk_storage.bytes_consumed + total_entity_block_size <= chunk_storage.chunk_byte_budget)
	{
		auto* new_block = storage_metadata_array.AddNew();
		if (!new_block)
			fatal_error("FATAL: Memory budget for World Chunk with id = %i has ended. Could not allocate memory for entity with type_id = %i.", id, type_id); 
		
		EntityStorageBlockMetadata& entity_storage_metadata = *new_block;
		entity_storage_metadata.type_id = type_id;
		entity_storage_metadata.type_size =  sizeof(T_Entity);
		entity_storage_metadata.max_entity_instances = T_Entity::instance_budget;
		entity_storage_metadata.entity_traits = T_Entity::traits.Copy();
		
		entity_storage_metadata.data_start = chunk_storage.next_block_start;
		chunk_storage.next_block_start += total_entity_block_size;
		chunk_storage.bytes_consumed += total_entity_block_size;
		storage_metadata_map[type_id] = &entity_storage_metadata;
	}
	else
		fatal_error("FATAL: Memory budget for World Chunk with id = %i has ended. Could not allocate memory for entity with type_id = %i.", id, type_id); 
};

template<typename T_Entity>
T_Entity* WorldChunk::RequestEntityStorage()
{
	MaybeAllocateForType<T_Entity>();

	if (auto** it = Find(storage_metadata_map, T_Entity::GetTypeId()))
	{
		EntityStorageBlockMetadata* block_metadata = *it;
		if (block_metadata->entity_count < block_metadata->max_entity_instances)
		{
			byte* entity_storage_address = block_metadata->data_start + (block_metadata->entity_count * block_metadata->type_size);
			++block_metadata->entity_count;
			auto* new_entity = new (entity_storage_address) T_Entity;
        
			return new_entity;
		}
		else
			print("ERROR: There is no memory budget left in WorldChunk with id = %i for E_Type with id = %i. Could not allocate memory.", id, T_Entity::GetTypeId());
	}
	else
		fatal_error("FATAL : This shouldn't have happened.");

	return nullptr;
}

template<typename T_Entity>
T_Entity* T_World::CreateEntity(u8 i, u8 j, u8 k)
{
	auto chunk_it = chunks_map.find({i, j, k});
	if (chunk_it == chunks_map.end())
		return nullptr;
	
	auto* chunk = chunk_it->second;
	auto* entity = chunk->RequestEntityStorage<T_Entity>();
	return entity;
}
