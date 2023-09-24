#pragma once

#include "engine/core/core.h"

void AN_AnimatePlayer(EPlayer* player);
void ForceInterruptPlayerAnimation(EPlayer* player);
bool AN_UpdatePlayerJumpingAnimation(EPlayer* player);
bool AN_UpdatePlayerLandingAnimation(EPlayer* player);
bool AN_UpdatePlayerLandingFallAnimation(EPlayer* player);
bool AN_PlayerVaulting(EPlayer* player);
