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
	static constexpr int size = 64;
	int count = 0;
	Entity* entities[size]{};
	u64 deferred_entity_ids[size]{};
	SrEntityRelation relations[size]{};
	u32 aux_uint_buffer[size]{};
};

struct EntitySerializer
{
	inline static EntityManager* manager = nullptr;
	inline static DeferredEntityRelationBuffer relations{};

	static void Parse(Parser& parser);
	static void Save(std::ofstream& writer, Entity& entity);

	static void ClearBuffer();
};
