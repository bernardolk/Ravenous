#pragma once

#include "engine/core/core.h"

void AN_animate_player(Player* player);
void AN_p_anim_force_interrupt(Player* player);
bool AN_p_anim_jumping_update(Player* player);
bool AN_p_anim_landing_update(Player* player);
bool AN_p_anim_landing_fall_update(Player* player);
bool AN_p_anim_vaulting(Player* player);