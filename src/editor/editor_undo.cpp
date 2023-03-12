#include "editor_undo.h"

#include "engine/rvn.h"
#include "engine/entities/entity.h"

namespace Editor
{
	void DeletedEntityLog::Add(const Entity* entity)
	{
		if (size + 1 == capacity)
		{
			Rvn::rm_buffer->Add("DeletedEntityLog is FULL!", 3000);
			return;
		}

		entity_ids[size++] = entity->id;
	};

	void UndoStack::Track(Entity* entity)
	{
		auto state = EntityState{
		entity,
		entity->id,
		entity->position,
		entity->scale,
		entity->rotation
		};

		Track(state);
	}

	void UndoStack::Track(EntityState state)
	{
		//log(LOG_INFO, "Tracking entity '" + state.entity->name + "'.");

		if (full)
		{
			Rvn::rm_buffer->Add("UNDO/REDO STACK FULL.", 800);
			return;
		}

		if (!compare_entity_states(state, Check()))
		{
			stack[++pos] = state;
			limit = pos;
		}
		full = IsBufferFull();
	}

	void UndoStack::Undo()
	{
		if (pos == 0)
			return;

		// gets a valid state to undo
		EntityState state;
		do
		{
			state = GetStateAndMoveBack();
			if (pos == 1 && !IsStateValid(state))
				return;
		} while (!IsStateValid(state));

		apply_state(state);
	}

	void UndoStack::Redo()
	{
		if (pos == 0)
			return;

		// gets a valid state to redo
		EntityState state;
		do
		{
			state = GetStateAndMoveUp();
			if (pos == limit && !IsStateValid(state))
				return;
		} while (!IsStateValid(state));

		apply_state(state);
	}

	EntityState UndoStack::Check()
	{
		if (pos > 0)
			return stack[pos];
		return EntityState{};
	}

	// internal
	EntityState UndoStack::GetStateAndMoveBack()
	{
		if (pos > 1)
			return stack[--pos];
		if (pos == 1)
			return stack[pos];
		return EntityState{};
	}

	// internal
	EntityState UndoStack::GetStateAndMoveUp()
	{
		if (pos < limit)
			return stack[++pos];
		if (pos == limit)
			return stack[pos];
		return EntityState{};
	}

	// internal
	bool UndoStack::IsBufferFull()
	{
		return limit + 1 == capacity;
	}

	// internal
	bool UndoStack::IsStateValid(EntityState state)
	{
		// if entity was deleted, it isnt valid
		for (int i = 0; i < deletion_log.size; i++)
			if (deletion_log.entity_ids[i] == state.id)
				return false;

		// if entity current state is equal to state in stack
		// then is not valid for undo also
		return !compare_entity_states(get_entity_state(state.entity), state);
	}
};
