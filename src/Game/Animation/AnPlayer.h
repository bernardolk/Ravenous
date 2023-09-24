#pragma once

#include "engine/core/core.h"

void AN_AnimatePlayer(Player* player);
void ForceInterruptPlayerAnimation(Player* player);
bool AN_UpdatePlayerJumpingAnimation(Player* player);
bool AN_UpdatePlayerLandingAnimation(Player* player);
bool AN_UpdatePlayerLandingFallAnimation(Player* player);
bool AN_PlayerVaulting(Player* player);
