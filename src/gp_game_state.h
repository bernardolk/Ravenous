
struct GameState {

   // timed events
   u32         timer_targets_array_size = 64;
   TimerTarget timer_targets[timer_targets_array_size];

   void start_timer(Entity* interactable)
   {
      For(timer_targets_array_size)
      {
         auto target = timer_targets[i];
         if(!target.active)
         {
            target.start_timer(interactable->target, interactable->timer_duration);  
            break;
         }
      }
   }


} Game_State;

