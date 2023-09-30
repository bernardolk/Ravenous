#pragma once

#include "engine/core/core.h"

struct RGameInputKey
{
	static inline uint64 MoveForward;
	static inline uint64 MoveBackward;
	static inline uint64 MoveLeft;
	static inline uint64 MoveRight;
	static inline uint64 Dash;
	static inline uint64 Walk;
	static inline uint64 Action;
};

inline uint64 KeyMoveUp, KeyMoveDown, KeyMoveLeft, KeyMoveRight, KeyDash, KeyWalk, KeyAction;

void InHandleMovementInput(struct RInputFlags Flags, EPlayer* Player, RWorld* World);
void InAssignKeysToActions();
void InProcessMoveKeys(RInputFlags Flags, vec3& VDir, bool ShortCircuit = false);

bool Pressed(RInputFlags Flags, uint64 Key);
bool PressedOnly(RInputFlags Flags, uint64 Key);
bool PressedOnce(RInputFlags Flags, uint64 Key);