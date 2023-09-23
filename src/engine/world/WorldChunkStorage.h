#pragma once

#include "engine/core/core.h"
#include "engine/entities/manager/entity_traits_manager.h"

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
	static inline constexpr u32 max_types_per_storage = 20;

	byte data[chunk_byte_budget];
	byte* next_block_start;
	u32 bytes_consumed = 0;
	int parent_chunk_id;

	// Holds the useful metadata about each storage block in chunk storage. Iterate over it to go through all entity types.
	Array<EntityStorageBlockMetadata, max_types_per_storage> storage_metadata_array;
	// Convenience map to retrieve storage metadata for a specific entity type.
	map<TypeID, EntityStorageBlockMetadata*> storage_metadata_map;
	// TODO: Implement a TraitID to array of typeIds

	ChunkStorage(int chunk_id) : parent_chunk_id(chunk_id)
	{
		next_block_start = &data[0];
		storage_metadata_array = Array<EntityStorageBlockMetadata, max_types_per_storage>();
	}
	
	template<typename T_Entity>
	T_Entity* ChunkStorage::RequestEntityStorage();

private:
	template<typename T_Entity>
	void ChunkStorage::MaybeAllocateForType();
};


struct WorldChunkEntityIterator
{
	WorldChunk* chunk;
	u32 block_idx = 0;
	u32 entity_idx = 0;
	
	explicit WorldChunkEntityIterator(WorldChunk* chunk) : chunk(chunk){}

	E_Entity* operator()();
};


template<typename T_Entity>
void ChunkStorage::MaybeAllocateForType()
{
	auto type_id = T_Entity::GetTypeId();

	if (storage_metadata_map.contains(type_id))
		return;

	// if we have budget, get a block for the new entity type
	u32 total_entity_block_size = T_Entity::instance_budget * sizeof(T_Entity);
	if(bytes_consumed + total_entity_block_size <= chunk_byte_budget)
	{
		auto* new_block = storage_metadata_array.AddNew();
		if (!new_block)
			fatal_error("FATAL: Memory budget for World Chunk with id = %i has ended. Could not allocate memory for entity with type_id = %i.", parent_chunk_id, type_id); 
		
		EntityStorageBlockMetadata& entity_storage_metadata = *new_block;
		entity_storage_metadata.type_id = type_id;
		entity_storage_metadata.type_size =  sizeof(T_Entity);
		entity_storage_metadata.max_entity_instances = T_Entity::instance_budget;
		entity_storage_metadata.entity_traits = T_Entity::traits.Copy();
		
		entity_storage_metadata.data_start = next_block_start;
		next_block_start += total_entity_block_size;
		bytes_consumed += total_entity_block_size;
		storage_metadata_map[type_id] = &entity_storage_metadata;
	}
	else
		fatal_error("FATAL: Memory budget for World Chunk with id = %i has ended. Could not allocate memory for entity with type_id = %i.", parent_chunk_id, type_id); 
};

template<typename T_Entity>
T_Entity* ChunkStorage::RequestEntityStorage()
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
			print("ERROR: There is no memory budget left in WorldChunk with id = %i for E_Type with id = %i. Could not allocate memory.", parent_chunk_id, T_Entity::GetTypeId());
	}
	else
		fatal_error("FATAL : This shouldn't have happened.");

	return nullptr;
}