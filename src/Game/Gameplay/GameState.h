#pragma once

#include "engine/core/core.h"
#include "game/animation/AnUpdate.h"
#include "gp_timer.h"

struct GameState
{
	// Timed events (timers)
	constexpr static u32 timers_array_size = 64;
	Timer timers[timers_array_size];

	EntityAnimationKeyframe tmp_kf;

	void StartTimer(Entity* trigger);
	void UpdateTimers();

	static GameState* Get()
	{
		static GameState instance;
		return &instance;
	}

private:
	GameState() = default;
};
