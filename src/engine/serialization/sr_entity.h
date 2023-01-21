#pragma once

struct T_EntityManager;
struct Entity;
struct Parser;
struct World;

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
	inline static T_EntityManager* manager = nullptr;
	inline static DeferredEntityRelationBuffer relations{};

	static void parse(Parser& parser);
	static void save(std::ofstream& writer, Entity& entity);

	static void _clear_buffer();
};
