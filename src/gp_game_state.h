
struct GameState {

   // Timed events (timers)
   const static size_t timers_array_size = 64;
   Timer timers[timers_array_size];

   EntityAnimationKeyframe tmp_kf;

   void start_timer(Entity* interactable)
   {
      For(timers_array_size)
      {
         auto timer = &timers[i];
         if(!timer->active)
         {
            timer->start(interactable->timer_target, interactable->timer_duration);  
            //@todo: tmp solution
            {
               tmp_kf.duration         = 2000;
               tmp_kf.starting_scale   = interactable->timer_target->scale;
               tmp_kf.final_scale      = vec3{-1, -1, 0.2};
               tmp_kf.flags            |= EntityAnimKfFlags_ChangeScale;

               Entity_Animations.start_animation(
                  interactable->timer_target,
                  &tmp_kf,
                  1
               );
            }
            return;
         }
      }

      Quit_fatal("Too many timer targets running at the same time.");
      return;
   }

   void update_timers()
   {
      For(timers_array_size)
      {
         auto timer = &timers[i];

         if(timer->active)
         {
            editor_print("Remaining time: " + fmt_tostr(timer->remaining_time, 0));
            timer->update();
         }
      }
   }

} Game_State;

