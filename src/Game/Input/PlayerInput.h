#pragma once

#include "engine/core/core.h"

inline uint64 KEY_MOVE_UP, KEY_MOVE_DOWN, KEY_MOVE_LEFT, KEY_MOVE_RIGHT, KEY_DASH, KEY_WALK, KEY_ACTION;

void IN_HandleMovementInput(struct RInputFlags flags, EPlayer* player, RWorld* world);
void IN_AssignKeysToActions();
void IN_ProcessMoveKeys(RInputFlags flags, vec3& v_dir, bool short_circuit = false);
