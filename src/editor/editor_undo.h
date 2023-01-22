#pragma once

struct DeletedEntityLog
{
	u8 size = 0;
	constexpr static u8 capacity = 100;
	int entity_ids[capacity];

	void Add(const Entity* entity)
	{
		if(size + 1 == capacity)
		{
			Rvn::rm_buffer->Add("DeletedEntityLog is FULL!", 3000);
			return;
		}

		entity_ids[size++] = entity->id;
	}
};

struct UndoStack
{
	u8 limit = 0;                   // index of last added item
	u8 pos = 0;                     // current index
	constexpr static u8 capacity = 100; // max items - 1 (pos = 0 is never assigned)
	EntityState stack[100];         // actual stack
	DeletedEntityLog deletion_log;  // stores ids of entities that have been deleted
	bool full = false;              // helps avoid writing out of stack mem boundaries

	void Track(Entity* entity)
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

	void Track(EntityState state)
	{
		//log(LOG_INFO, "Tracking entity '" + state.entity->name + "'.");

		if(full)
		{
			Rvn::rm_buffer->Add("UNDO/REDO STACK FULL.", 800);
			return;
		}

		if(!compare_entity_states(state, Check()))
		{
			stack[++pos] = state;
			limit = pos;
		}
		full = IsBufferFull();
	}

	void Undo()
	{
		if(pos == 0)
			return;

		// gets a valid state to undo
		EntityState state;
		do
		{
			state = GetStateAndMoveBack();
			if(pos == 1 && !IsStateValid(state))
				return;
		} while(!IsStateValid(state));

		apply_state(state);
	}

	void Redo()
	{
		if(pos == 0)
			return;

		// gets a valid state to redo
		EntityState state;
		do
		{
			state = GetStateAndMoveUp();
			if(pos == limit && !IsStateValid(state))
				return;
		} while(!IsStateValid(state));

		apply_state(state);
	}

	EntityState Check()
	{
		if(pos > 0)
			return stack[pos];
		return EntityState{};
	}

	// internal
	EntityState GetStateAndMoveBack()
	{
		if(pos > 1)
			return stack[--pos];
		if(pos == 1)
			return stack[pos];
		return EntityState{};
	}

	// internal
	EntityState GetStateAndMoveUp()
	{
		if(pos < limit)
			return stack[++pos];
		if(pos == limit)
			return stack[pos];
		return EntityState{};
	}

	// internal
	bool IsBufferFull()
	{
		return limit + 1 == capacity;
	}

	// internal
	bool IsStateValid(EntityState state)
	{
		// if entity was deleted, it isnt valid
		for(int i = 0; i < deletion_log.size; i++)
			if(deletion_log.entity_ids[i] == state.id)
				return false;

		// if entity current state is equal to state in stack
		// then is not valid for undo also
		return !compare_entity_states(get_entity_state(state.entity), state);
	}
};
