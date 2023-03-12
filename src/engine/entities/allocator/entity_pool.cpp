#include "entity_pool.h"
#include "engine/entities/entity.h"
#include "engine/core/logging.h"

void EntityPool::Init()
{
	// allocate pool memory
	pool = static_cast<Entity*>(malloc(size * sizeof(Entity)));
	if (pool == nullptr)
	{
		std::cout << "FATAL: failed to allocate memory for EntityPool.\n";
		assert(false);
	}

	For(size)
	{
		// initializes entity with "placement new"
		auto _ = new(pool + i) Entity();
		// marks entity as "empty"
		pool[i].flags |= EntityFlags_EmptyEntity;
	}
}

// @TODO: Refactor, this is stupid
[[nodiscard]] Entity* EntityPool::GetNext() const
{
	For(size)
	{
		if (pool[i].flags & EntityFlags_EmptyEntity)
		{
			pool[i].flags &= ~(EntityFlags_EmptyEntity);
			return &pool[i];
		}
	}

	std::cout << "EntityPool is full!\n";
	assert(false);
	return nullptr;
}

//@TODO: Refactor, this is stupid
void EntityPool::FreeSlot(const Entity* entity) const
{
	Entity* cursor = pool;
	For(size)
	{
		if (cursor->id == entity->id)
		{
			*cursor = Entity();
			cursor->flags |= EntityFlags_EmptyEntity;
			return;
		}
		cursor++;
	}

	log(LOG_WARNING, "Entity '" + entity->name + "' requested to be deleted couldn't be found in entity pool.");
}
