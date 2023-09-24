#pragma once

#include "engine/core/core.h"
#include "game/animation/AnUpdate.h"

struct RGameState
{
	DeclSingleton(RGameState)
	
	// Timed events (timers)
	constexpr static u32 timers_array_size = 64;

	REntityAnimationKeyframe tmp_kf;

	void StartTimer(EEntity* trigger);
	void UpdateTimers();
};
