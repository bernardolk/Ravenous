#pragma once

#include "engine/core/core.h"

void AnAnimatePlayer(EPlayer* Player);
void ForceInterruptPlayerAnimation(EPlayer* Player);
bool AnUpdatePlayerJumpingAnimation(EPlayer* Player);
bool AnUpdatePlayerLandingAnimation(EPlayer* Player);
bool AnUpdatePlayerLandingFallAnimation(EPlayer* Player);
bool AnPlayerVaulting(EPlayer* Player);
