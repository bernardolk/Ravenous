#pragma once

#include "engine/core/core.h"
#include "EntityState.h"

namespace Editor
{
	struct DeletedEntityLog
	{
		u8 size = 0;
		constexpr static u8 capacity = 100;
		int entity_ids[capacity];

	public:
		void Add(const EEntity* entity);
	};

	struct UndoStack
	{
		u8 limit = 0;                       // index of last added item
		u8 pos = 0;                         // current index
		constexpr static u8 capacity = 100; // max items - 1 (pos = 0 is never assigned)
		EntityState stack[100];             // actual stack
		DeletedEntityLog deletion_log;      // stores ids of entities that have been deleted
		bool full = false;                  // helps avoid writing out of stack mem boundaries

	public:
		void Track(EEntity* entity);
		void Track(EntityState state);
		void Undo();
		void Redo();
		EntityState Check();
		// internal
		EntityState GetStateAndMoveBack();
		// internal
		EntityState GetStateAndMoveUp();
		// internal
		bool IsBufferFull();
		// internal
		bool IsStateValid(EntityState state);
	};
}
