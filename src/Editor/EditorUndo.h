#pragma once

#include "engine/core/core.h"
#include "EntityState.h"

namespace Editor
{
	struct RDeletedEntityLog
	{
		uint8 Size = 0;
		constexpr static uint8 Capacity = 100;
		int EntityIds[Capacity];

		void Add(const EEntity* Entity);
	};

	struct RUndoStack
	{
		uint8 Limit = 0;                       // index of last added item
		uint8 Pos = 0;                         // current index
		constexpr static uint8 Capacity = 100; // max items - 1 (pos = 0 is never assigned)
		REntityState Stack[100];               // actual stack
		RDeletedEntityLog DeletionLog;        // stores ids of entities that have been deleted
		bool Full = false;                     // helps avoid writing out of stack mem boundaries

		void Track(EEntity* Entity);
		void Track(REntityState State);
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
		bool IsStateValid(REntityState State);
	};
}
