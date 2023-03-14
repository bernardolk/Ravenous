#pragma once

#include "engine/core/core.h"
#include "engine/entities/base_entity.h"
#include "engine/entities/manager/entity_traits_manager.h"

/*
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

	// ChunkStorage()
	// {
	// 	for (int i = 0; i < chunk_byte_budget; i ++)
	// 		data[i] = '\0';
	// 	
	// 	next_block_start = &data[0];
	// }
};

struct WorldChunk
{
    using byte = char;

	// TODO: World chunk ID needs to come from it's world position
    static inline u32 world_chunk_id_count = 0;
	static inline constexpr size_t max_types_per_storage = 20;
	
    unsigned int id = ++world_chunk_id_count;

	// Low level byte array
	ChunkStorage chunk_storage;
	// Holds the useful metadata about each storage block in chunk storage. Iterate over it to go through all entity types.
	Array<EntityStorageBlockMetadata, max_types_per_storage> storage_metadata_array;
	// Convenience map to retrieve storage metadata for a specific entity type.
    map<TypeID, EntityStorageBlockMetadata*> storage_metadata_map;
	// TODO: Implement a TraitID to array of typeIds
	
    template<typename T_Entity>
    Iterator<T_Entity*> GetIterator()
    {
        return Iterator<T_Entity*>({}, 0);
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

/** World */

struct T_World
{
	// in the future such vector will be replaced with a memory arena
	Array<WorldChunk, 2> chunks;
	vector<WorldChunk*> active_chunks;

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

private:
	T_World(){};
};
