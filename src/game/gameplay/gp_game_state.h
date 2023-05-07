#pragma once

#include "engine/core/core.h"
#include "game/animation/an_update.h"
#include "gp_timer.h"

struct T_GameState
{
	// Timed events (timers)
	constexpr static u32 timers_array_size = 64;
	Timer timers[timers_array_size];

	EntityAnimationKeyframe tmp_kf;

	void StartTimer(Entity* trigger);
	void UpdateTimers();
};

//Todo: refactor global
inline T_GameState GameState;
