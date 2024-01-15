#pragma once

#include "engine/core/core.h"
#include "EntityState.h"
#include "Engine/Core/RView.h"
#include "Engine/Entities/EHandle.h"

// =====================================
//	Undo Stack
// =====================================
// Our Undo stack model assumes that the top of the stack represents the current state of the entity tracked.
// That model implies that each tracked state change will have its initial and a final state tracked.
// We only track a state if the last tracked state isn't the same as the one being tracked.
// Moving an entity, we track the transforms before and after. Deleting, we track spawning then deleting, etc.
// Due to that, we need to check if it is valid to apply the state before we undo / redo.
// E.g. if you move something then delete it, and move something else, when you undo, we should skip the deletion statechange and go straight into the spawning statechange.
// So, for every new tracked action, be sure to add both initial (whatever it may be) and final states.
// Also, not a problem to have lefover initial states after undoing then performing another different action, since we will always check if applying the state is valid.
// E.g.
//		Action #1 Deletion  -> Stack = [Spawn] -> [Delete]
//													^
//		Action #2 Undo		-> Stack = [Spawn] -> [Delete]
//										  ^				
//		Action #1 Deletion  -> Stack = [Spawn] -> [Spawn] -> [Delete]
//																^
// When undoing from that last delete, we are just going to spawn the entity once, that's a guarantee of the system


namespace Editor
{
	
enum class NEntityStateChangeType
{
	EmptyStateChange,
	TransformChange,
	Deletion,
	Creation,
};

struct REntityStateChange
{
	REntityState State;
	NEntityStateChangeType Type = NEntityStateChangeType::EmptyStateChange;
	RView<REntityStateChange> InverseStateChange;
};

struct RUndoStack
{
	int64 Pos = -1;
	vector<REntityStateChange> Stack;

	void TrackTransformChange(const EHandle<EEntity>& Entity);
	void TrackDeletion(const EHandle<EEntity>& Entity);
	void TrackCreation(const EHandle<EEntity>& Entity);
	void Undo();
	void Redo();
	REntityStateChange GetStateChangeAtPosition();
	
private:
	RView<REntityStateChange> Track(const EHandle<EEntity>& Entity, NEntityStateChangeType Type);
	bool ApplyStateChange(REntityStateChange& StateChange);
	bool AreTrackedStatesEqual(const REntityStateChange& State1, const REntityStateChange& State2);
	void EraseForwardHistoryIfOnMiddleOfTheStack();
};

bool IsEntityDeleted(RUUID ID);
}
