#include "EditorUndo.h"

#include "engine/rvn.h"
#include "engine/entities/Entity.h"

namespace Editor
{
	void RDeletedEntityLog::Add(const EEntity* entity)
	{
		if (size + 1 == capacity)
		{
			Rvn::rm_buffer->Add("DeletedEntityLog is FULL!", 3000);
			return;
		}

		entity_ids[size++] = entity->id;
	};

	void RUndoStack::Track(EEntity* entity)
	{
		auto state = REntityState{
		entity,
		entity->id,
		entity->position,
		entity->scale,
		entity->rotation
		};

		Track(state);
	}

	void RUndoStack::Track(REntityState state)
	{
		//log(LOG_INFO, "Tracking entity '" + state.entity->name + "'.");

		if (full)
		{
			Rvn::rm_buffer->Add("UNDO/REDO STACK FULL.", 800);
			return;
		}

		if (!CompareEntityStates(state, Check()))
		{
			stack[++pos] = state;
			limit = pos;
		}
		full = IsBufferFull();
	}

	void RUndoStack::Undo()
	{
		if (pos == 0)
			return;

		// gets a valid state to undo
		REntityState state;
		do
		{
			state = GetStateAndMoveBack();
			if (pos == 1 && !IsStateValid(state))
				return;
		} while (!IsStateValid(state));

		ApplyState(state);
	}

	void RUndoStack::Redo()
	{
		if (pos == 0)
			return;

		// gets a valid state to redo
		REntityState state;
		do
		{
			state = GetStateAndMoveUp();
			if (pos == limit && !IsStateValid(state))
				return;
		} while (!IsStateValid(state));

		ApplyState(state);
	}

	REntityState RUndoStack::Check()
	{
		if (pos > 0)
			return stack[pos];
		return REntityState{};
	}

	// internal
	REntityState RUndoStack::GetStateAndMoveBack()
	{
		if (pos > 1)
			return stack[--pos];
		if (pos == 1)
			return stack[pos];
		return REntityState{};
	}

	// internal
	REntityState RUndoStack::GetStateAndMoveUp()
	{
		if (pos < limit)
			return stack[++pos];
		if (pos == limit)
			return stack[pos];
		return REntityState{};
	}

	// internal
	bool RUndoStack::IsBufferFull()
	{
		return limit + 1 == capacity;
	}

	// internal
	bool RUndoStack::IsStateValid(REntityState state)
	{
		// if entity was deleted, it isnt valid
		for (int i = 0; i < deletion_log.size; i++)
			if (deletion_log.entity_ids[i] == state.id)
				return false;

		// if entity current state is equal to state in stack
		// then is not valid for undo also
		return !CompareEntityStates(GetEntityState(state.entity), state);
	}
};
