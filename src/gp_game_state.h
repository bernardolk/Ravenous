struct GameState
{

	// Timed events (timers)
	const static size_t timers_array_size = 64;
	Timer               timers[timers_array_size];

	EntityAnimationKeyframe tmp_kf;

	void start_timer(Entity* trigger)
	{
		For(timers_array_size)
		{
			auto timer = &timers[i];
			if(!timer->active)
			{
				timer->start(trigger->timer_trigger_data.timer_target, trigger, trigger->timer_trigger_data.timer_duration);

				// plays animation, if entity has one
				if(timer->target->timer_target_data.timer_start_animation != 0)
				{
					auto anim = &Animation_Catalogue.find(timer->target->timer_target_data.timer_start_animation)->second;
					Entity_Animations.start_animation(timer->target, anim);
				}

				return;
			}
		}

		Quit_fatal("Too many timer targets running at the same time.");
	}

	void update_timers()
	{
		For(timers_array_size)
		{
			auto timer = &timers[i];

			if(timer->active)
			{
				RVN::print_dynamic("Remaining time: " + fmt_tostr(timer->remaining_time, 0));
				bool active = timer->update();
				if(!active)
				{

					// plays animation, if entity has one
					if(timer->target->timer_target_data.timer_stop_animation != 0)
					{
						auto anim = &Animation_Catalogue.find(timer->target->timer_target_data.timer_stop_animation)->second;
						Entity_Animations.start_animation(timer->target, anim);
					}

					timer->stop();
				}
			}
		}
	}

} Game_State;
