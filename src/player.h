enum PlayerStateEnum {
   PLAYER_STATE_FALLING,
   PLAYER_STATE_STANDING,
   PLAYER_STATE_WALKING,
   PLAYER_STATE_RUNNING,
   PLAYER_STATE_SPRINTING,
   PLAYER_STATE_JUMPING,
   PLAYER_STATE_SLIDING,
   PLAYER_STATE_SLIDE_FALLING,
   PLAYER_STATE_GRABBING,
   PLAYER_STATE_FALLING_FROM_EDGE,
   PLAYER_STATE_EVICTED_FROM_SLOPE
};

struct Player {
   Entity* entity_ptr;
   Entity* standing_entity_ptr;
   Entity* slope_player_was_ptr;

   float speed = 1.5f;
   float fall_speed = 0.01f;
   float fall_acceleration = 0.2f;
   float jump_initial_speed = 5.0f;
   float slide_jump_speed = 8.0f;
   float slide_speed = 1.5f;
   float radius; 
   float half_height;

   PlayerStateEnum player_state;
   PlayerStateEnum initial_player_state;

   vec3 prior_position = vec3(0);
   vec3 initial_velocity = vec3(0);

   int lives = 2;
   float height_before_fall;
   float hurt_height_1 = 3.5;
   float hurt_height_2 = 5.0;

   vec3 feet()
   {
      return entity_ptr->position - vec3(0.0f, half_height, 0.0f);
   }
};