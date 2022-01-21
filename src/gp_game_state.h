
struct GameState {

   // timed event
   bool     timer_active            = false;
   Entity*  timer_target            = nullptr;
   float    timer_remaining_time    = 0;

   // events
   u32 events;

} Game_State;


// prototypes
void GP_update_game_state();
void GP_game_state_stop_timer();
void GP_game_state_start_timer(Entity* interactable);



void GP_update_game_state()
{
   if(Game_State.timer_active)
   {
      Game_State.timer_remaining_time -= G_FRAME_INFO.duration;
      if(Game_State.timer_remaining_time <= 0)
         GP_game_state_stop_timer();
   }
}


void GP_game_state_stop_timer()
{
   Game_State.timer_active = false;
   Game_State.timer_target->timer_end_action = true;
}


void GP_game_state_start_timer(Entity* interactable)
{
   if(Game_State.timer_target == nullptr)
      return;
      
   Game_State.timer_remaining_time              = (float) interactable->timer_duration;
   Game_State.timer_target                      = interactable->timer_target;
   Game_State.timer_active                      = true;
   Game_State.timer_target->timer_end_action    = true;
}


