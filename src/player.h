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
   float speed = 1.0f;
   float fall_speed = 0.0f;
   float fall_acceleration = 0.1f;
   float jump_initial_speed = 4.0f;
   float slide_speed = 1.0f;
   float fall_from_edge_speed = 2.0f;
   PlayerStateEnum player_state;
   float radius;
   float half_height;
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

