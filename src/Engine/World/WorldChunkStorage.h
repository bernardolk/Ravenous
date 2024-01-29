#pragma once

#include "engine/core/core.h"
#include "Engine/Entities/Traits/EntityTraitsManager.h"

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
struct REntityStorageBlockMetadata
{
	using byte = char;

	// type data
	REntityTypeID TypeID;
	uint TypeSize;
	// TODO: We don't need to know about Traits at this level, lets only deal with entity Types
	Array<RTraitID, EntityTraitsManager::MaxTraits> EntityTraits;

	// array bookkeeping
	uint MaxEntityInstances;
	int EntityCount = 0;
	byte* DataStart = nullptr;
};

/** Memory Arena for storing all the world chunk's entities's data. */
struct RWorldChunkStorage
{
	using byte = char;

	// MAX BUDGET for ALL entities
	inline static constexpr uint ChunkByteBudget = 666000;
	static inline constexpr uint MaxTypesPerStorage = 20;

	byte Data[ChunkByteBudget];
	byte* NextBlockStart;
	uint BytesConsumed = 0;
	int ParentChunkID;

	// Holds the useful metadata about each storage block in chunk storage. Iterate over it to go through all entity types.
	Array<REntityStorageBlockMetadata, MaxTypesPerStorage> StorageMetadataArray;
	// Convenience map to retrieve storage metadata for a specific entity type.
	map<REntityTypeID, REntityStorageBlockMetadata*> StorageMetadataMap;
	// TODO: Implement a TraitID to array of typeIds

	RWorldChunkStorage(int ChunkId) :
		ParentChunkID(ChunkId)
	{
		NextBlockStart = &Data[0];
		StorageMetadataArray = Array<REntityStorageBlockMetadata, MaxTypesPerStorage>();
	}

	template<typename TEntity>
	TEntity* RequestEntityStorage();

private:
	template<typename TEntity>
	void MaybeAllocateForType();
};


struct RWorldChunkEntityIterator
{
	RWorldChunk* Chunk = nullptr;
	uint BlockIdx = 0;
	uint EntityIdx = 0;

	explicit RWorldChunkEntityIterator(RWorldChunk* Chunk) :
		Chunk(Chunk) {}

	EEntity* operator()();
};


template<typename TEntity>
void RWorldChunkStorage::MaybeAllocateForType()
{
	REntityTypeID TypeId = TEntity::GetTypeID();

	if (StorageMetadataMap.contains(TypeId))
		return;

	// if we have budget, get a block for the new entity type
	uint TotalEntityBlockSize = TEntity::InstanceBudget * sizeof(TEntity);
	if (BytesConsumed + TotalEntityBlockSize <= ChunkByteBudget)
	{
		auto* NewBlock = StorageMetadataArray.AddNew();
		if (!NewBlock)
			FatalError("FATAL: Memory budget for World Chunk with id = %i has ended. Could not allocate memory for entity with TypeId = %i.", ParentChunkID, TypeId);

		REntityStorageBlockMetadata& EntityStorageMetadata = *NewBlock;
		EntityStorageMetadata.TypeID = TypeId;
		EntityStorageMetadata.TypeSize = sizeof(TEntity);
		EntityStorageMetadata.MaxEntityInstances = TEntity::InstanceBudget;
		EntityStorageMetadata.EntityTraits = TEntity::Traits.Copy();

		EntityStorageMetadata.DataStart = NextBlockStart;
		NextBlockStart += TotalEntityBlockSize;
		BytesConsumed += TotalEntityBlockSize;
		StorageMetadataMap[TypeId] = &EntityStorageMetadata;
	}
	else
		FatalError("FATAL: Memory budget for World Chunk with id = %i has ended. Could not allocate memory for entity with TypeId = %i.", ParentChunkID, TypeId);
};

template<typename TEntity>
TEntity* RWorldChunkStorage::RequestEntityStorage()
{
	MaybeAllocateForType<TEntity>();

	if (auto** it = Find(StorageMetadataMap, TEntity::GetTypeID()))
	{
		REntityStorageBlockMetadata* BlockMetadata = *it;
		if (BlockMetadata->EntityCount < BlockMetadata->MaxEntityInstances)
		{
			byte* EntityStorageAddress = BlockMetadata->DataStart + (BlockMetadata->EntityCount * BlockMetadata->TypeSize);
			++BlockMetadata->EntityCount;
			auto* NewEntity = new (EntityStorageAddress) TEntity;
        
			return NewEntity;
		}
		else {
			Log("ERROR: There is no memory budget left in RWorldChunk with id = %i for E_Type with id = %i. Could not allocate memory.", ParentChunkID, TEntity::GetTypeID());
		}
	}
	else {
		FatalError("FATAL : This shouldn't have happened.");
	}

	return nullptr;
}
