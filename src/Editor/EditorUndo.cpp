#include "EditorUndo.h"

#include "Reflection/Serialization.h"
#include "engine/entities/Entity.h"
#include "tools/EditorTools.h"

namespace Editor
{
	
void RUndoStack::TrackTransformChange(const EHandle<EEntity>& Entity)
{
	EraseForwardHistoryIfOnMiddleOfTheStack();
	Track(Entity, NEntityStateChangeType::TransformChange);
}

void RUndoStack::TrackDeletion(const EHandle<EEntity>& Entity)
{
	EraseForwardHistoryIfOnMiddleOfTheStack();
	auto Initial = Track(Entity, NEntityStateChangeType::Creation);
	auto Final = Track(Entity, NEntityStateChangeType::Deletion);
	Initial->InverseStateChange = Final;
	Final->InverseStateChange = Initial;
}

void RUndoStack::TrackCreation(const EHandle<EEntity>& Entity)
{
	EraseForwardHistoryIfOnMiddleOfTheStack();
	auto Initial = Track(Entity, NEntityStateChangeType::Deletion);
	auto Final = Track(Entity, NEntityStateChangeType::Creation);
	Initial->InverseStateChange = Final;
	Final->InverseStateChange = Initial;
}

RView<REntityStateChange> RUndoStack::Track(const EHandle<EEntity>& Entity, NEntityStateChangeType Type)
{
	REntityStateChange StateChange;
	StateChange.State = GetEntityState(Entity);
	StateChange.Type = Type;
	
	if (!AreTrackedStatesEqual(StateChange, GetStateChangeAtPosition())) {
		Stack.push_back(StateChange); ++Pos;
	}
	return {&Stack, &Stack.back()};
}

void RUndoStack::Undo()
{
	if (Pos <= 0) return;
	while (!ApplyStateChange(Stack[--Pos])) {
		// Shouldn't ever reach this if no bugs exists since there will always be a valid rollback state if Pos > 0 at first iteration
		if (Pos == 0) return;
	};
}

void RUndoStack::Redo()
{
	if (Pos == Stack.size() - 1) return;
	while (!ApplyStateChange(Stack[++Pos])) {
		// Shouldn't ever reach this if no bugs exists since there will always be a valid rollback state if Pos > 0 at first iteration
		if (Pos == Stack.size() - 1) return;
	};
}

bool RUndoStack::ApplyStateChange(REntityStateChange& StateChange)
{
	switch (StateChange.Type)
	{
		case NEntityStateChangeType::TransformChange:
		{
			if (!StateChange.State.Entity.IsValid())
			{
				auto NewHandle = MakeHandleFromID(StateChange.State.ID);
				if (!NewHandle.IsValid()) {
					Log("Error RUndo::ApplyStateChange: There was a state change in stack with an entity that was not found."); DEBUG_BREAK
					return false;
				}
				StateChange.State.Entity = NewHandle;
			}
				
			if (!AreEntityStatesEqual(GetEntityState(StateChange.State.Entity), StateChange.State)) {
				StateChange.State.Apply();
				GetContext()->bGizmoPositionsDirty = true;
				return true;
			}
			return false;
		}
		case NEntityStateChangeType::Creation:
		{
			// Only restore entity if it was deleted in this editor session
			auto& DeletionLog = REditorContext::Get()->DeletionLog;
			for (auto It = DeletionLog.begin(); It != DeletionLog.end(); ++It)
			{
				if (StateChange.State.ID == *It)
				{
					auto* RestoredEntity = Serialization::LoadEntityFromFile(*It);
					if (!RestoredEntity) {
						Break("Couldn't Restore entity through undo/redo. Save work and restart editor to prevent corruption of undo stack.")
						return false;
					}
					DeletionLog.erase(It);
					// Update this state change and related one handles of entity to point to the new entity slot
					auto Handle = MakeHandle<EEntity>(RestoredEntity);
					StateChange.State.Entity = Handle;
					StateChange.InverseStateChange.Get()->State.Entity = Handle;
					StateChange.State.Apply();
					return true;
				}	
			}
			return false;
		}
		case NEntityStateChangeType::Deletion:
		{
			// Only apply state change of deletion if this is not that entity's current final state. That is, the entity isnt currently deleted in editor session.
			for (auto& ID : REditorContext::Get()->DeletionLog) {
				if (StateChange.State.ID == ID) {
					return false;
				}
			}

			// If the entity is not deleted, update handle if necessary (find new entity location in memory since it could have been moved around)
			if (!StateChange.State.Entity.IsValid())
			{
				auto NewHandle = MakeHandleFromID(StateChange.State.ID);
				if (!NewHandle.IsValid()) {
					Log("Error RUndo::ApplyStateChange: There was a state change in stack with an entity that was not found."); DEBUG_BREAK
					return false;
				}
			}
				
			EditorDeleteEntity(StateChange.State.Entity, true);
			return true;
		}
	}
}

REntityStateChange RUndoStack::GetStateChangeAtPosition()
{
	return Pos > -1 ? Stack[Pos] : REntityStateChange{};
}

bool RUndoStack::AreTrackedStatesEqual(const REntityStateChange& State1, const REntityStateChange& State2)
{
	return State1.Type == State2.Type && AreEntityStatesEqual(State1.State, State2.State);
}

void RUndoStack::EraseForwardHistoryIfOnMiddleOfTheStack()
{
	if (Pos < Stack.size() - 1) {
		Stack.erase(Stack.begin() + Pos + 1, Stack.end());
	}
}

bool IsEntityDeleted(RUUID ID)
{
	auto* EdContext = REditorContext::Get();
	for (RUUID DeletedID : EdContext->DeletionLog) {
		if (ID == DeletedID) {
			return true;			
		}
	}

	return false;
}
	
}
