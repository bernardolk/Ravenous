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

   float speed = 3.0f;
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

   int initial_lives = 2;
   int lives = 2;
   float height_before_fall;
   float hurt_height_1 = 3.5;
   float hurt_height_2 = 5.0;

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