#pragma once

#include "engine/core/core.h"
#include "game/animation/AnUpdate.h"

struct RGameState
{
	static RGameState* Get() 
	{ 
		static RGameState Instance{};
		return &Instance;
	}
	
	// Timed events (timers)
	constexpr static uint TimersArraySize = 64;

	REntityAnimationKeyframe TmpKf;

	void StartTimer(EEntity* Trigger);
	void UpdateTimers();
};
