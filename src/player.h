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
   float speed = 1.0f;
   float fall_speed = 1.0f;
   float fall_acceleration = 0.1f;
   float jump_initial_speed = 4.0f;
   float slide_speed = 2.5f;
   float fall_from_edge_speed = 2.0f;
   PlayerStateEnum player_state;
   float radius;
   float half_height;
   vec3 prior_position = vec3(0);
   vec3 initial_velocity = vec3(0);
   PlayerStateEnum initial_player_state;
};