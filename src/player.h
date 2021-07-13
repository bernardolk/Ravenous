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

   // movement variables
   vec3 v_dir = vec3(0.f);
   float speed = 0;
   float acceleration = 7.5;
   float air_delta_speed = 0.05;
   float run_speed = 4.0;
   float dash_speed = 6.0;
   float fall_speed = 0.01;
   float fall_acceleration = 0.2;
   float jump_initial_speed = 5.0;
   float jump_horz_thrust = 2.5;
   float slide_jump_speed = 8.0;
   float slide_speed = 3.0;
   float radius;
   float half_height;

   // movement states
   bool dashing = false;
   bool jumping_upwards = false;
   bool landing = false;

   PlayerStateEnum player_state;
   PlayerStateEnum initial_player_state;

   vec3 prior_position = vec3(0);
   vec3 initial_velocity = vec3(0);

   // health and hurting
   int initial_lives = 2;
   int lives = 2;
   float height_before_fall;
   float hurt_height_1 = 5.0;
   float hurt_height_2 = 8.0;

   // set when checking for fall, read-only!
   float fall_height_log = 0;

   Entity* checkpoint = nullptr;
   vec3 checkpoint_pos;

   vec3 feet()
   {
      return entity_ptr->position - vec3(0.0f, half_height, 0.0f);
   }

   bool maybe_hurt_from_fall()
   {
      float fall_height = height_before_fall - entity_ptr->position.y;
      fall_height_log = fall_height;
      if(fall_height >= hurt_height_2)
      {
         lives -= 2;
         return true;
      }
      else if(fall_height >= hurt_height_1)
      {
         lives -= 1;
         return true;
      }
      return false;
   }

   void restore_health()
   {
      lives = initial_lives;
   }

   void set_checkpoint(Entity* entity)
   {
      if(entity->type != CHECKPOINT) assert(false);

      checkpoint_pos = entity_ptr->position;
      checkpoint = entity;
   }

   void goto_checkpoint()
   {
      if(checkpoint == nullptr) cout << "teleporting player to initial position.\n";
      entity_ptr->position = checkpoint_pos;
   }

   void die()
   {
      goto_checkpoint();
      lives = initial_lives;
   }
};