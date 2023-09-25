#pragma once

#include "engine/core/core.h"

inline uint64 KeyMoveUp, KeyMoveDown, KeyMoveLeft, KeyMoveRight, KeyDash, KeyWalk, KeyAction;

void InHandleMovementInput(struct RInputFlags Flags, EPlayer* Player, RWorld* World);
void InAssignKeysToActions();
void InProcessMoveKeys(RInputFlags Flags, vec3& VDir, bool ShortCircuit = false);
