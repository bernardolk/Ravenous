#pragma once

#include "engine/collision/raycast.h"
#include "engine/core/core.h"
#include "engine/entities/e_base_entity.h"
#include "engine/entities/manager/entity_traits_manager.h"
#include "engine/utils/utils.h"

struct E_Entity;
auto WorldCoordsToCells(float x, float y, float z);
vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k);

// how many cells we have preallocated for the world
constexpr static int WCellsNumX = 4;
constexpr static int WCellsNumY = 4;
constexpr static int WCellsNumZ = 4;

// how many cells are before and after the origin in each axis
constexpr static int WCellsOffsetX = WCellsNumX / 2;
constexpr static int WCellsOffsetY = WCellsNumY / 2;
constexpr static int WCellsOffsetZ = WCellsNumZ / 2;

// how many meters the cell occupies in the world
constexpr static float WCellLenMeters = 50.0f;
// how many entities can coexists in a cell
constexpr static int WorldCellCapacity = 150;

const static vec3 WUpperBoundsMeters = {
	WCellsOffsetX * WCellLenMeters,
	WCellsOffsetY * WCellLenMeters,
	WCellsOffsetZ * WCellLenMeters
	};

const static vec3 WLowerBoundsMeters = {
	-1.0 * WCellsOffsetX * WCellLenMeters,
	-1.0 * WCellsOffsetX * WCellLenMeters,
	-1.0 * WCellsOffsetX * WCellLenMeters
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
	size_t type_size;
	// TODO: We don't need to know about traits at this level, lets only deal with entity Types
	Array<TraitID, EntityTraitsManager::max_traits> entity_traits{};

	// array bookkeeping
	size_t max_entity_instances;
	int entity_count = 0;
	byte* data_start = nullptr;
};

/** Memory Arena for storing all the world chunk's entities's data. */
struct ChunkStorage
{
	using byte = char;
	
	inline static constexpr size_t chunk_byte_budget = 1200;
	
	byte data[chunk_byte_budget]{};
	byte* next_block_start = &data[0];
	u32 bytes_consumed = 0;
};

struct WorldChunkPosition
{
	i32 i = -1, j = -1, k = -1;
	
	bool operator == (const WorldChunkPosition& other)
	{
		return other.i == i && other.j == j && other.k == k;
	}

	bool operator == (const vec3& other)
	{
		return other.x == i && other.y == j && other.z == k;
	}
};

struct VisitorState
{
	bool visiting = false;
	WorldChunkPosition chunk_position{};
	struct WorldChunk* chunk_ptr = nullptr;

	void Reset() { new (this) VisitorState(); }
};

struct WorldChunk
{
    using byte = char;

	friend struct WorldChunkEntityIterator;
	
	// TODO: World chunk ID needs to come from it's world position
    static inline u32 world_chunk_id_count = 0;
	static inline constexpr size_t max_types_per_storage = 20;
	
    unsigned int id = ++world_chunk_id_count;

	u32 i = -1, j = -1, k = -1;

	// Low level byte array
	ChunkStorage chunk_storage;
	// Holds the useful metadata about each storage block in chunk storage. Iterate over it to go through all entity types.
	Array<EntityStorageBlockMetadata, max_types_per_storage> storage_metadata_array;
	// Convenience map to retrieve storage metadata for a specific entity type.
    map<TypeID, EntityStorageBlockMetadata*> storage_metadata_map;
	// TODO: Implement a TraitID to array of typeIds

	vector<E_Entity*> visitors{};
	
	
	WorldChunk(u32 i, u32 j, u32 k) : i(i), j(j), k(k) {}

	WorldChunkEntityIterator GetIterator();

	WorldChunkPosition GetPosition() { return WorldChunkPosition(i, j, k); }

	void RemoveEntity(E_Entity* entity);

	bool AddVisitor(E_Entity* entity);
	bool RemoveVisitor(E_Entity* entity);

	vec3 GetPositionMetric()
	{
		return GetWorldCoordinatesFromWorldCellCoordinates(i, j , k);
	}
	
	WorldChunkPosition GetChunkPosition() { return WorldChunkPosition(i,j,k); }

	string GetChunkPositionString()
	{
		return "Cell [" + to_string(i) + "," + to_string(j) + "," + to_string(k) + "]";
	}

	string GetChunkPositionMetricString()
	{
		vec3 mcoords = GetPositionMetric();
		return "[x: " + FormatFloatTostr(mcoords[0], 1)
		+ ", y: " + FormatFloatTostr(mcoords[1], 1) + ", z: " + FormatFloatTostr(mcoords[2], 1)
		+ "]";
	}
	
	
	// TODO: We should refactor this method so it uses a list of TypeIDs from EntityTraitsManager so that we only check the chunk's TypeID to EntityStorageBlockMetadata map
	// TODO: This way the world chunk storage doesn't need to care about traits at all.
    void InvokeTraitUpdateOnAllTypes(TraitID trait_id)
    {
        auto* etm = EntityTraitsManager::Get();
        for (auto& block_metadata : storage_metadata_array)
        {
            if (Contains(block_metadata.entity_traits, trait_id))
            {
                auto* trait_update_func = etm->GetUpdateFunc(block_metadata.type_id, trait_id);
                for (int i = 0; i < block_metadata.entity_count; i++)
                {
                    auto* entity = reinterpret_cast<E_BaseEntity*>(block_metadata.data_start + block_metadata.type_size * i);
                    trait_update_func(entity);
                }
            }
        }
    }


    template<typename T_Entity>
	void MaybeAllocateForType()
    {
        auto type_id = T_Entity::GetTypeId();

    	if (storage_metadata_map.contains(type_id))
    		return;
    	
        EntityStorageBlockMetadata entity_storage_metadata;
    	entity_storage_metadata.type_id = type_id;
    	entity_storage_metadata.type_size =  sizeof(T_Entity);
    	entity_storage_metadata.max_entity_instances = T_Entity::instance_budget;
        entity_storage_metadata.entity_traits = T_Entity::traits.Copy();
        
        // if we have budget, get a block for the new entity type
    	u32 total_entity_block_size = entity_storage_metadata.max_entity_instances * entity_storage_metadata.type_size;
        if(chunk_storage.bytes_consumed + total_entity_block_size <= chunk_storage.chunk_byte_budget)
        {
            entity_storage_metadata.data_start = chunk_storage.next_block_start;
            chunk_storage.next_block_start += total_entity_block_size;
        	chunk_storage.bytes_consumed += total_entity_block_size;
            storage_metadata_map[type_id] = storage_metadata_array.Add(entity_storage_metadata);
        }
        else
        {
            std::cout << "FATAL: Memory budget for World Chunk with id = " << id << " has ended. Could not allocate memory for entity with type_id = " << type_id << "\n"; 
            assert(false);
        }
    };

    template<typename T_Entity>
	T_Entity* RequestEntityStorage()
    {
        MaybeAllocateForType<T_Entity>();

        if (auto** it = Find(storage_metadata_map, T_Entity::GetTypeId()))
        {
        	EntityStorageBlockMetadata* block_metadata = *it;
            if (block_metadata->entity_count < block_metadata->max_entity_instances)
            {
                byte* entity_storage_address = (block_metadata->data_start + block_metadata->entity_count++ * block_metadata->type_size);
            	auto* new_entity = new (entity_storage_address) T_Entity();
            	
                return new_entity;
            }
            else
            {
                std::cout << "ERROR: There is no memory budget left in WorldChunk with id = " << id << " for E_Type with id = " << T_Entity::GetTypeId() << ". Could not allocate memory.\n";
            }
        }
        else
        {
            std::cout << "FATAL : This shouldn't have happened.\n";
            assert(false);
        }
    	
        return nullptr;
    }
};

struct WorldChunkEntityIterator
{
	WorldChunk* chunk;
	u32 block_idx = 0;
	u32 entity_idx = 0;
	
	explicit WorldChunkEntityIterator(WorldChunk* chunk) : chunk(chunk){};

	E_Entity* operator()()
	{
		if (block_idx < chunk->storage_metadata_array.count)
		{
			auto* block_metadata = chunk->storage_metadata_array.GetAt(block_idx);
			if (entity_idx < block_metadata->entity_count)
			{
				return reinterpret_cast<E_Entity*>(block_metadata->data_start + block_metadata->type_size * entity_idx++);
			}

			entity_idx = 0;
			block_idx++;
		}
		return nullptr;
	};
};

/** World */

struct T_World
{
	// static constexpr u8 world_chunk_matrix_order = 10;
	static constexpr u32 world_size_in_chunks = WCellsNumX * WCellsNumY * WCellsNumZ;

	// TODO: We can't use world chunk "matrix" position as its ijk position! This is insane! What if we want to unload part A of the world and load part B,
	// what are the index going to say? Nothing.
	// in the future such vector will be replaced with a memory arena
	Array<WorldChunk, world_size_in_chunks> chunks;
	map<WorldChunkPosition, WorldChunk*> chunks_map;
	vector<WorldChunk*> active_chunks;

	Player* player = nullptr;

	float global_shininess = 17;
	float ambient_intensity = 0;
	vec3 ambient_light = vec3(1);

	// temp
	vector<PointLight*> point_lights;
	vector<SpotLight*> spot_lights;
	vector<DirectionalLight*> directional_lights;
	// end temp
		
public:
	void Update()
	{
		auto* entity_traits_manager = EntityTraitsManager::Get();
		for (TraitID trait_id : entity_traits_manager->entity_traits)
		{
			for (auto* chunk: active_chunks)
			{
				chunk->InvokeTraitUpdateOnAllTypes(trait_id);
			} 
		}
	}

	static T_World* Get()
	{
		static T_World instance;
		return &instance;
	};


	template<typename T_Entity>
	T_Entity* CreateEntity(u8 i, u8 j, u8 k)
	{
		auto chunk_it = chunks_map.find({i, j, k});
		if (chunk_it != chunks_map.end())
			return nullptr;
		
		auto* chunk = chunk_it->second;
		auto* entity = chunk->RequestEntityStorage<T_Entity>();
		return entity;
	}

	Iterator<WorldChunk> GetIterator();
	
	RaycastTest Raycast(Ray ray, RayCastType test_type, const E_Entity* skip = nullptr, float max_distance = MaxFloat) const;
	RaycastTest Raycast(Ray ray, const E_Entity* skip = nullptr, float max_distance = MaxFloat) const;
	RaycastTest LinearRaycastArray(Ray first_ray, int qty, float spacing) const;
	RaycastTest RaycastLights(Ray ray) const;

	CellUpdate UpdateEntityWorldCells(E_Entity* entity);
	
private:
	T_World();
};

inline vec3 GetWorldCoordinatesFromWorldCellCoordinates(int i, int j, int k)
{
	const float world_x = (static_cast<float>(i) - WCellsOffsetX) * WCellLenMeters;
	const float world_y = (static_cast<float>(j) - WCellsOffsetY) * WCellLenMeters;
	const float world_z = (static_cast<float>(k) - WCellsOffsetZ) * WCellLenMeters;

	return vec3{world_x, world_y, world_z};
}

inline auto WorldCoordsToCells(float x, float y, float z)
{
	struct
	{
		int i = -1, j = -1, k = -1;
	} world_cell_coords;

	// if out of bounds return -1
	if (x < WLowerBoundsMeters.x || x > WUpperBoundsMeters.x ||
		y < WLowerBoundsMeters.y || y > WUpperBoundsMeters.y ||
		z < WLowerBoundsMeters.z || z > WUpperBoundsMeters.z)
	{
		return world_cell_coords;
	}

	// int division to truncate float result to correct cell position
	world_cell_coords.i = (x + WCellsOffsetX * WCellLenMeters) / WCellLenMeters;
	world_cell_coords.j = (y + WCellsOffsetY * WCellLenMeters) / WCellLenMeters;
	world_cell_coords.k = (z + WCellsOffsetZ * WCellLenMeters) / WCellLenMeters;

	return world_cell_coords;
}

inline auto WorldCoordsToCells(vec3 position)
{
	return WorldCoordsToCells(position.x, position.y, position.z);
}


//TODO: Move these somewhere else
void SetEntityDefaultAssets(E_Entity* entity);
void SetEntityAssets(E_Entity* entity, struct EntityAttributes attrs);