
struct GameState {

   // timed events
   const static size_t timer_targets_array_size = 64;
   TimerTarget timer_targets[timer_targets_array_size];

   void start_timer(Entity* interactable)
   {
      For(timer_targets_array_size)
      {
         auto timer = &timer_targets[i];
         if(!timer->active)
         {
            timer->start(interactable->timer_target, interactable->timer_duration);  
            break;
         }
      }
   }
} Game_State;


void GP_update_timers()
{
   For(Game_State.timer_targets_array_size)
   {
      auto timer = &Game_State.timer_targets[i];

      if(timer->active)
      {
         // perform timer target action according to entity timer target type
         switch(timer->entity->timer_target_type)
         {
            case EntityTimerTargetType_VerticalSlidingDoor:
            {
               // animation code call
               

               editor_print("Remaining time: " + fmt_tostr(timer->remaining_time, 0));
               break;
            }
         }

         // update timer
         timer->update();
      }
   }
}