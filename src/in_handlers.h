#pragma once

#include "engine/core/core.h"

inline u64 KEY_MOVE_UP, KEY_MOVE_DOWN, KEY_MOVE_LEFT, KEY_MOVE_RIGHT, KEY_DASH, KEY_WALK, KEY_ACTION;

struct InputFlags;
void IN_handle_common_input(InputFlags flags, Player* & player);
void IN_handle_movement_input(InputFlags flags, Player* & player, World* world);
void IN_assign_keys_to_actions();
void IN_process_move_keys(InputFlags flags, vec3& v_dir);
