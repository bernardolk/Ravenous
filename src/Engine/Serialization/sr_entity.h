#pragma once

#include "engine/core/core.h"

struct Parser;

enum SrEntityRelation
{
	SrEntityRelation_TimerTarget  = 0,
	SrEntityRelation_TimerMarking = 1
};

// Allows storing relationships between parsed entities to be set after parsing is done
struct DeferredEntityRelationBuffer
{
	static constexpr int Size = 64;
	int count = 0;
	EEntity* entities[Size];
	uint64 deferred_entity_ids[Size];
	SrEntityRelation relations[Size];
	uint aux_uint_buffer[Size];
};

struct EntitySerializer
{
	inline static EntityManager* Manager = nullptr;
	inline static DeferredEntityRelationBuffer Relations{};

	static void Parse(Parser& Parser);
	static void Save(std::ofstream& Writer, const EEntity& Entity);

	static void ClearBuffer();
};
