#include "EditorUndo.h"

#include "engine/rvn.h"
#include "engine/entities/Entity.h"

namespace Editor
{
	void RDeletedEntityLog::Add(const EEntity* Entity)
	{
		if (Size + 1 == Capacity)
		{
			PrintEditorMsg("DeletedEntityLog is FULL!");
			return;
		}

		EntityIds[Size++] = Entity->ID;
	};

	void RUndoStack::Track(EEntity* Entity)
	{
		Track(REntityState{Entity, Entity->ID, Entity->Position, Entity->Scale, Entity->Rotation});
	}

	void RUndoStack::Track(REntityState State)
	{
		//log(LOG_INFO, "Tracking entity '" + state.entity->name + "'.");

		if (Full)
		{
			PrintEditorMsg("UNDO/REDO STACK FULL");
			return;
		}

		if (!CompareEntityStates(State, Check()))
		{
			Stack[++Pos] = State;
			Limit = Pos;
		}
		Full = IsBufferFull();
	}

	void RUndoStack::Undo()
	{
		if (Pos == 0)
			return;

		// gets a valid state to undo
		REntityState State;
		do
		{
			State = GetStateAndMoveBack();
			if (Pos == 1 && !IsStateValid(State))
				return;
		} while (!IsStateValid(State));

		ApplyState(State);
	}

	void RUndoStack::Redo()
	{
		if (Pos == 0)
			return;

		// gets a valid state to redo
		REntityState State;
		do
		{
			State = GetStateAndMoveUp();
			if (Pos == Limit && !IsStateValid(State))
				return;
		} while (!IsStateValid(State));

		ApplyState(State);
	}

	REntityState RUndoStack::Check()
	{
		if (Pos > 0)
			return Stack[Pos];
		return REntityState{};
	}

	// internal
	REntityState RUndoStack::GetStateAndMoveBack()
	{
		if (Pos > 1)
			return Stack[--Pos];
		if (Pos == 1)
			return Stack[Pos];
		return REntityState{};
	}

	// internal
	REntityState RUndoStack::GetStateAndMoveUp()
	{
		if (Pos < Limit)
			return Stack[++Pos];
		if (Pos == Limit)
			return Stack[Pos];
		return REntityState{};
	}

	// internal
	bool RUndoStack::IsBufferFull()
	{
		return Limit + 1 == Capacity;
	}

	// internal
	bool RUndoStack::IsStateValid(REntityState State)
	{
		// if entity was deleted, it isnt valid
		for (int I = 0; I < DeletionLog.Size; I++)
			if (DeletionLog.EntityIds[I] == State.ID)
				return false;

		// if entity current state is equal to state in stack
		// then is not valid for undo also
		return !CompareEntityStates(GetEntityState(State.Entity), State);
	}
};
