enum PlayerStateEnum {
   PLAYER_STATE_FALLING,
   PLAYER_STATE_STANDING,
   PLAYER_STATE_WALKING,
   PLAYER_STATE_RUNNING,
   PLAYER_STATE_SPRINTING,
   PLAYER_STATE_JUMPING,
   PLAYER_STATE_SLIDING,
   PLAYER_STATE_GRABBING,
   PLAYER_STATE_FALLING_FROM_EDGE
};

struct Player {
   Entity* entity_ptr;
   Entity* standing_entity_ptr;
   float speed = 0.6f;
   float fall_speed = 0.8f;
   float fall_from_edge_speed = 0.02f;
   PlayerStateEnum player_state;
};


// hmmm ....
void player_change_state(Player* player, PlayerStateEnum new_state)
{
   switch(new_state)
   {
      case PLAYER_STATE_FALLING:

      break;
      case PLAYER_STATE_STANDING:

      break;
   }

}

