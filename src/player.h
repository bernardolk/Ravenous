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

// ----------
// Animation
// ----------
enum PlayerAnimationState {
   P_ANIM_NO_ANIM          = 999,
   P_ANIM_JUMPING          = 0,
   P_ANIM_LANDING          = 1,
   P_ANIM_LANDING_FALL     = 2
};

float P_ANIM_DURATION[] = {
   400,                          // 0 - jumping
   200,                          // 1 - landing                  
   400                           // 2 - landing fall   
};

// forward declarations
struct Player;
void p_anim_force_interrupt(Player* player);

struct Player {
   Entity* entity_ptr;
   Entity* standing_entity_ptr;
   Entity* slope_player_was_ptr;

   // movement variables
   vec3 v_dir = vec3(0.f);
   float speed = 0;

   // movement constants
   float acceleration = 7.5;
   float air_delta_speed = 0.05;
   float run_speed = 4.0;
   float dash_speed = 6.0;
   float fall_speed = 0.01;
   float fall_acceleration = 0.2;
   float jump_initial_speed = 5.0;
   float jump_horz_thrust = 2.5;
   float slide_jump_speed = 6.7;
   float slide_speed = 2.0;
   float radius;
   float half_height;
   
   // movement states
   bool dashing = false;
   bool jumping_upwards = false;
   bool landing = false;
   bool jumping_from_slope = false;

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

   // checkpoints
   Entity* checkpoint = nullptr;
   vec3 checkpoint_pos;

   // animation
   float anim_t = 0;             // animation timer
   PlayerAnimationState anim_state = P_ANIM_NO_ANIM; 

   vec3 feet()
   {
      return entity_ptr->position - vec3(0.0f, half_height, 0.0f);
   }

   vec3 top()
   {
      return entity_ptr->position + vec3(0.0f, half_height, 0.0f);
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
      entity_ptr->position = checkpoint_pos;
   }

   void die()
   {
      lives = initial_lives;
      entity_ptr->velocity = vec3(0);
      player_state = PLAYER_STATE_STANDING;
      p_anim_force_interrupt(this);
      goto_checkpoint();
   }

   void start_jump_animation()
   {

   }
};