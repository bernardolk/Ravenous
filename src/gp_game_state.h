#pragma once

// ReSharper disable once CppInconsistentNaming
inline struct T_GameState
{
	// Timed events (timers)
	const static size_t timers_array_size = 64;
	Timer timers[timers_array_size];

	EntityAnimationKeyframe tmp_kf;

	void StartTimer(Entity* trigger)
	{
		For(timers_array_size)
		{
			auto timer = &timers[i];
			if(!timer->active)
			{
				timer->Start(trigger->timer_trigger_data.timer_target, trigger, trigger->timer_trigger_data.timer_duration);

				// plays animation, if entity has one
				if(timer->target->timer_target_data.timer_start_animation != 0)
				{
					auto anim = &AnimationCatalogue.find(timer->target->timer_target_data.timer_start_animation)->second;
					EntityAnimations.StartAnimation(timer->target, anim);
				}

				return;
			}
		}

		Quit_fatal("Too many timer targets running at the same time.");
	}

	void UpdateTimers()
	{
		For(timers_array_size)
		{
			auto timer = &timers[i];

			if(timer->active)
			{
				Rvn::PrintDynamic("Remaining time: " + FmtTostr(timer->remaining_time, 0));
				bool active = timer->Update();
				if(!active)
				{

					// plays animation, if entity has one
					if(timer->target->timer_target_data.timer_stop_animation != 0)
					{
						auto anim = &AnimationCatalogue.find(timer->target->timer_target_data.timer_stop_animation)->second;
						EntityAnimations.StartAnimation(timer->target, anim);
					}

					timer->Stop();
				}
			}
		}
	}

} GameState;
