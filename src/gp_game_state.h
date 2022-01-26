
struct GameState {

   // timed events
   u32         timer_targets_array_size = 64;
   TimerTarget timer_targets[timer_targets_array_size];

} Game_State;


// prototypes
void GP_game_state_stop_timer();
void GP_game_state_start_timer(Entity* interactable);



void GP_game_state_stop_timer()
{
   Game_State.timer_active = false;
   Game_State.timer_target->timer_end = true;
}


void GP_game_state_start_timer(Entity* interactable)
{

}


