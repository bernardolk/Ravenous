
enum GameEvents {
   GameEvent_TimerStart = 1 << 0,
   GameEvent_TimerEnd   = 1 << 1,
};


struct GameState {

   // timed event
   bool     timer_active            = false;
   Entity*  timer_target            = nullptr;
   float    timer_remaining_time    = 0;

   // events
   u32 events;

} Game_State;


void GP_game_state_reset_events()
{
   Game_State.events = 0;
}



void GP_update_game_state()
{
   if(Game_State.timer_active)
      GP_game_state_update_timed_event();
}


void GP_game_state_update_timed_event()
{
   if(Game_State.events & GameEvent_TimerStart)
      Game_State.timer_target->timer_start_action = true;

   Game_State.timer_remaining_time -= G_FRAME_INFO.duration;
   if(Game_State.timer_remaining_time <= 0)
   {
      Game_State.timer_active = false;
      Game_State.events |= GameEvent_TimerEnd;
   }
}

//@todo
void GP_game_state_start_timer()
{

}


