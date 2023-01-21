enum TimerType
{
	TimerType_Simple         = 0,
	TimerType_TimeAttackDoor = 1,
};

struct Timer
{
	TimerType type = TimerType_Simple;
	Entity*   target = nullptr;
	Entity*   trigger = nullptr;
	bool      active = false;
	float     remaining_time = 0;
	float     elapsed_time = 0;

	void start(Entity* target, Entity* trigger, float duration)
	{
		this->target = target;
		this->trigger = trigger;
		remaining_time = duration;
		elapsed_time = 0;
		active = true;

		// @todo: could be any kind of time_attack_door, but ok.
		if(target->timer_target_data.timer_target_type == EntityTimerTargetType_VerticalSlidingDoor)
		{
			type = TimerType_TimeAttackDoor;
			_start_time_attack_door_timer();
		}
	}

	void stop()
	{
		// specific stop procedures first
		if(type == TimerType_TimeAttackDoor)
			_stop_time_attack_door_timer();

		target = nullptr;
		trigger = nullptr;
		active = false;
		remaining_time = 0;
		elapsed_time = 0;
		type = TimerType_Simple;
	}

	bool update()
	{
		/* Returns whether the timer is still active */
		remaining_time -= RVN::frame.duration;
		elapsed_time += RVN::frame.duration;

		if(type == TimerType_TimeAttackDoor)
			_update_time_attack_door_timer();

		if(remaining_time <= 0)
			return false;
		return true;
	}


	void _start_time_attack_door_timer()
	{
		// not sure this should live here and not in the entity.h file. But ok.
		auto data = &trigger->timer_trigger_data;
		For(data->size)
		{
			data->notification_mask[i] = false;
			auto entity = data->markings[i];

			// turns  every marking on
			if(entity != nullptr)
				entity->timer_marking_data.color = entity->timer_marking_data.color_on;
		}
	}


	void _update_time_attack_door_timer()
	{
		auto data = &trigger->timer_trigger_data;
		For(data->size)
		{
			auto entity = data->markings[i];
			if(entity != nullptr && data->notification_mask[i] == false && data->time_checkpoints[i] <= elapsed_time)
			{
				// turn the marking off
				entity->timer_marking_data.color = entity->timer_marking_data.color_off;
				data->notification_mask[i] = true;
			}
		}
	}

	void _stop_time_attack_door_timer()
	{
		auto data = &trigger->timer_trigger_data;
		For(data->size)
		{
			auto entity = data->markings[i];
			if(entity != nullptr)
			{
				// turns all markings off
				entity->timer_marking_data.color = entity->timer_marking_data.color_off;
				data->notification_mask[i] = false;
			}
		}
	}
};
