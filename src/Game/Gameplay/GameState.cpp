#include "GameState.h"
#include "engine/utils/utils.h"

void RGameState::StartTimer(EEntity* Trigger)
{
	/**
	For(timers_array_size)
	{
		auto timer = &timers[i];
		if (!timer->active)
		{
			timer->Start(trigger->timer_trigger_data.timer_target, trigger, trigger->timer_trigger_data.timer_duration);

			// plays animation, if entity has one
			if (timer->target->timer_target_data.timer_start_animation != 0)
			{
				auto anim = &AnimationCatalogue.find(timer->target->timer_target_data.timer_start_animation)->second;
				EntityAnimations.StartAnimation(timer->target, anim);
			}

			return;
		}
	}

	Quit_fatal("Too many timer targets running at the same time.");
	*/
}

void RGameState::UpdateTimers()
{
	/**
	For(timers_array_size)
	{
		auto timer = &timers[i];

		if (timer->active)
		{
			Rvn::PrintDynamic("Remaining time: " + FmtTostr(timer->remaining_time, 0));
			bool active = timer->Update();
			if (!active)
			{

				// plays animation, if entity has one
				if (timer->target->timer_target_data.timer_stop_animation != 0)
				{
					auto anim = &AnimationCatalogue.find(timer->target->timer_target_data.timer_stop_animation)->second;
					EntityAnimations.StartAnimation(timer->target, anim);
				}

				timer->Stop();
			}
		}
	}
	*/
}
