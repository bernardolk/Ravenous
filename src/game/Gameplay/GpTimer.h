#pragma once

#include "engine/core/core.h"

enum TimerType
{
	TimerType_Simple         = 0,
	TimerType_TimeAttackDoor = 1,
};

struct Timer
{
	TimerType type = TimerType_Simple;
	Entity* target = nullptr;
	Entity* trigger = nullptr;
	bool active = false;
	float remaining_time = 0;
	float elapsed_time = 0;

	void Start(Entity* target, Entity* trigger, float duration);
	void Stop();
	bool Update();

private:
	void StartTimeAttackDoorTimer();
	void UpdateTimeAttackDoorTimer();
	void StopTimeAttackDoorTimer();
};
