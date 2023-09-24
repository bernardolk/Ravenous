#pragma once

#include "engine/core/core.h"
#include "EntityState.h"

namespace Editor
{
	struct RDeletedEntityLog
	{
		uint8 size = 0;
		constexpr static uint8 capacity = 100;
		int entity_ids[capacity];

		void Add(const EEntity* entity);
	};

	struct RUndoStack
	{
		uint8 limit = 0;                       // index of last added item
		uint8 pos = 0;                         // current index
		constexpr static uint8 capacity = 100; // max items - 1 (pos = 0 is never assigned)
		REntityState stack[100];             // actual stack
		RDeletedEntityLog deletion_log;      // stores ids of entities that have been deleted
		bool full = false;                  // helps avoid writing out of stack mem boundaries

		void Track(EEntity* entity);
		void Track(REntityState state);
		void Undo();
		void Redo();
		REntityState Check();
		// internal
		REntityState GetStateAndMoveBack();
		// internal
		REntityState GetStateAndMoveUp();
		// internal
		bool IsBufferFull();
		// internal
		bool IsStateValid(REntityState state);
	};
}
