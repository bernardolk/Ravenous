#pragma once

#include "engine/core/core.h"
#include "EntityState.h"

namespace Editor
{
	struct RUndoStack
	{
		uint8 Limit = 0;						// index of last added item
		uint8 Pos = 0;							// current index
		constexpr static uint8 Capacity = 100;	// max items - 1 (pos = 0 is never assigned)
		REntityState Stack[100];				// actual stack
		bool Full = false;						// helps avoid writing out of stack mem boundaries

		void Track(EEntity* Entity);
		void Track(REntityState State);
		void Undo();
		void Redo();
		REntityState Check();

	private:
		REntityState GetStateAndMoveBack();
		REntityState GetStateAndMoveUp();
		bool IsBufferFull();
		bool IsStateValid(const REntityState& State);
	};
}
