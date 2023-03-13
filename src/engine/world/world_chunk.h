#pragma once

#include "engine/core/core.h"
#include "engine/entities/base_entity.h"
#include "engine/entities/manager/entity_traits_manager.h"

struct WorldChunk
{
    using byte = char;

    static inline size_t world_chunk_id_count = 0;
    unsigned int id = ++world_chunk_id_count;

    struct MemArena
    {
        inline static constexpr size_t budget = 1200;
        byte data[budget];
        byte* next_array_start = &data[0];
        unsigned int bytes_consumed;
    } memory;

    struct EntityMemArray
    {
        // type data
        int type_id;
        int type_size;
        int entity_traits;

        // array bookkeeping
        int size;
        int count = 0;
        byte* array;
    };

    map<int, EntityMemArray*> type_to_mem_array;
    Array<EntityMemArray, 20> entity_mem_arrays;

    template<typename T_Entity>
    Iterator<T_Entity*> GetIterator()
    {
        return Iterator<T_Entity*>({}, 0);
    }

    void InvokeTraitUpdateOnAllTypes(unsigned int trait_id)
    {
        auto* etm = EntityTraitsManager::Get();
        for(auto& mem_array : entity_mem_arrays)
        {
            if(mem_array.entity_traits & trait_id)
            {
                auto* trait_update_f = etm->GetUpdateFunc(mem_array.type_id, trait_id);
                int i = 0;
                while(i < mem_array.count)
                {
                    auto* entity_ptr = reinterpret_cast<E_BaseEntity*>(mem_array.array + mem_array.type_size * i);
                    trait_update_f(entity_ptr);
                    i++;
                }
            }
        }
    }

    template<typename T_Entity>
	void MaybeAllocateForType()
    {
        auto type_id = T_Entity::GetTypeId();

        // if we don't have a memory array for this type yet ...
        if(!type_to_mem_array.contains(type_id))
        { 
            EntityMemArray m;
            m.type_id = type_id;
            m.type_size = sizeof(T_Entity);
            m.size = T_Entity::memory_budget;
            m.entity_traits = T_Entity::traits;
            
            // if we have budget, get a new memory array for the new type
            if(memory.bytes_consumed + m.size * m.type_size <= memory.budget)
            {
                m.array = memory.next_array_start;
                memory.next_array_start += m.size * m.type_size;

                auto* mem_array_slot = entity_mem_arrays.Add(m);
                type_to_mem_array[type_id] = mem_array_slot;
            }
            else
            {
                std::cout << "FATAL: Memory budget for World Chunk with id = " 
                << id 
                << " has ended. Could not allocate memory for entity with type_id = "
                << type_id
                << "\n"; 

                assert(false);
            }
        }
    };

    template<typename T_Entity>
	T_Entity* RequestEntityStorage()
    {
        MaybeAllocateForType<T_Entity>();

        if(auto it = type_to_mem_array.find(T_Entity::GetTypeId()); it != type_to_mem_array.end())
        {
            EntityMemArray* mem_array = it->second;
            if(mem_array->count < mem_array->size)
            {
                byte* entity_storage_address = (mem_array->array + ++mem_array->count * mem_array->type_size);
                auto zeroed_memory = new (entity_storage_address) T_Entity();
                return zeroed_memory;
            }
            else
            {
                std::cout << "ERROR: There is no memory budget left in WorldChunk with id = " 
                << id 
                << " for E_Type with id = " << T_Entity::GetTypeId() 
                << ". Could not allocate memory.\n";
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

/**                 World               */

struct World
{
	// in the future such vector will be replaced with a memory arena
	std::vector<WorldChunk> chunks;
	std::vector<WorldChunk*> active_chunks;

	void Update()
	{
		auto* entity_traits_manager = EntityTraitsManager::Get();
		for(TraitID trait_id : entity_traits_manager->entity_traits)
		{
			for (auto* chunk: active_chunks)
			{
				chunk->InvokeTraitUpdateOnAllTypes(trait_id);
			} 
		}
	}

	static World* Get()
	{
		static World instance;
		return &instance;
	};

private:
	World(){};
};
