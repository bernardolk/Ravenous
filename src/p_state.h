
/* 
   --------------------
   > PLAYER STATE CODE
   --------------------
   Work in progress state machine-like modelling of player state 
*/

void P_state_change_jumping_to_falling    (Player* player);
void P_state_change_standing_to_falling   (Player* player);
void P_state_change_falling_to_standing   (Player* player);
void P_state_change_standing_to_jumping   (Player* player);
void P_state_change_standing_to_sliding   (Player* player, Entity* ramp);
void P_state_change_any_to_grabbing       (Player* player, Entity* entity, vec2 normal_vec, vec3 final_position, float d);
void P_state_change_grabbing_to_vaulting  (Player* player);
void P_state_change_standing_to_vaulting  (Player* player, Entity* entity, vec2 normal_vec, vec3 final_position);
void P_state_change_vaulting_to_standing  (Player* player);
void P_state_change_standing_to_slide_falling   (Player* player, Entity* ramp);


struct PlayerStateChangeArgs {

   // collision
   Entity* entity       = nullptr;
   vec3 normal          = vec3(0);
   float penetration    = 0;

   // grabbing info
   vec3 final_position  = vec3(0);

};

void P_change_state(Player* player, PlayerStateEnum new_state, PlayerStateChangeArgs args = {})
{
   /* This will change the player's state to the new state and do actions based on his current state.
      Hopefuly we can achieve a state machine model where all transitions are mapped and, therefore, predictable.
   */

  // Player is...

   // IN ANY STATE
   if(new_state == PLAYER_STATE_GRABBING)
      return P_state_change_any_to_grabbing(player, args.entity, args.normal, args.final_position, args.penetration);

   switch(player->player_state)
   {
      // STANDING
      case PLAYER_STATE_STANDING:
         switch(new_state)
         {
            case PLAYER_STATE_FALLING:
               P_state_change_standing_to_falling(player);
               break;
            case PLAYER_STATE_JUMPING:
               P_state_change_standing_to_jumping(player);
               break;
            case PLAYER_STATE_SLIDING:
               P_state_change_standing_to_sliding(player, args.entity);
               break;
            case PLAYER_STATE_SLIDE_FALLING:
               P_state_change_standing_to_slide_falling(player, args.entity);
               break;
            case PLAYER_STATE_VAULTING:
               P_state_change_standing_to_vaulting(player, args.entity, args.normal, args.final_position);
               break;
         }
         break;

      // JUMPING
      case PLAYER_STATE_JUMPING:
         switch(new_state)
         {
            case PLAYER_STATE_FALLING:
               P_state_change_jumping_to_falling(player);
               break;
         }
         break;

      // FALLING
      case PLAYER_STATE_FALLING:
         switch(new_state)
         {
            case PLAYER_STATE_STANDING:
               //P_state_change_falling_to_standing(player);
               break;
         }
         break;

      // GRABBING
      case PLAYER_STATE_GRABBING:
         switch(new_state)
         {
            case PLAYER_STATE_VAULTING:
               P_state_change_grabbing_to_vaulting(player);
               break;
         }
         break;

      // VAULTING
      case PLAYER_STATE_VAULTING:
         switch(new_state)
         {
            case PLAYER_STATE_STANDING:
               P_state_change_vaulting_to_standing(player);
               break;
         }
         break;

      default:
         assert(false);
   }
}


void P_state_change_jumping_to_falling(Player* player)
{
   player->player_state                = PLAYER_STATE_FALLING;
   player->entity_ptr->velocity.y      = 0;
   player->jumping_upwards             = false;

   // TEMP - DELETE LATER
   player->skip_collision_with_floor   = NULL;
}


void P_state_change_standing_to_jumping(Player* player)
{
   auto& v = player->entity_ptr->velocity;
   auto& v_dir = player->v_dir;
   bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

   if(no_move_command)
      player->jumping_upwards = true;
   else
   {
      // square_LE(v, player->jump_horz_thrust)
      if(player->dashing)
         v = v_dir * player->jump_horz_dash_thrust;
      else
         v = v_dir * player->jump_horz_thrust;
   }
   
   player->player_state = PLAYER_STATE_JUMPING;
   player->anim_state = P_ANIM_JUMPING;
   player->height_before_fall = player->entity_ptr->position.y;
   v.y = player->jump_initial_speed;

   // TEMP - DELETE LATER
   player->skip_collision_with_floor = player->standing_entity_ptr;
   player->standing_entity_ptr = nullptr;
}


void P_state_change_standing_to_falling(Player* player)
{
   player->player_state                   = PLAYER_STATE_FALLING;
   player->entity_ptr->velocity.y         = -1 * player->fall_speed;
   player->entity_ptr->velocity.x         *= 0.5;
   player->entity_ptr->velocity.z         *= 0.5;
   player->height_before_fall             = player->entity_ptr->position.y;
}


void P_state_change_falling_to_standing(Player* player, Entity* new_floor)
{
   // move player to surface, stop player and set him to standing
   player->standing_entity_ptr            = new_floor;
   auto height_check                      = CL_get_terrain_height_at_player(player->entity_ptr, player->standing_entity_ptr);
   player->entity_ptr->position.y         = height_check.overlap + player->half_height; 
   player->entity_ptr->velocity           = vec3(0);
   player->player_state                   = PLAYER_STATE_STANDING;

   // conditional animation: if falling from jump, land, else, land from fall
   if(player->half_height < P_HALF_HEIGHT)
      player->anim_state                  = P_ANIM_LANDING;
   else
      player->anim_state                  = P_ANIM_LANDING_FALL;

   player->maybe_hurt_from_fall();
}


// @TODO REFACTOR -> Refactor the CL
void P_state_change_standing_to_sliding(Player* player, Entity* ramp)
{
   player->standing_entity_ptr         = ramp;
   auto height_check                   = CL_get_terrain_height_at_player(player->entity_ptr, ramp);
   player->entity_ptr->position.y      = height_check.overlap + player->half_height;

   // make player 'snap' to slope velocity-wise
   player->entity_ptr->velocity = player->slide_speed * ramp->collision_geometry.slope.tangent;

   player->player_state = PLAYER_STATE_SLIDING;
}


// @TODO REFACTOR -> Refactor the CL
void P_state_change_standing_to_slide_falling(Player* player, Entity* ramp)
{
   player->standing_entity_ptr         = ramp;
   auto height_check                   = CL_get_terrain_height_at_player(player->entity_ptr, ramp);
   player->entity_ptr->position.y      = height_check.overlap + player->half_height;

   // make player 'snap' to slope velocity-wise
   player->entity_ptr->velocity = player->slide_speed * ramp->collision_geometry.slope.tangent;

   player->player_state = PLAYER_STATE_SLIDE_FALLING;
}

// @TODO REFACTOR -> From aabb and 2d normals to full 3d
void P_state_change_any_to_grabbing(Player* player, Entity* entity, vec2 normal_vec, vec3 final_position, float penetration)
{
   vec3 rev_normal = rev_2Dnormal(normal_vec);

   // this will be an animation in the future
   float turn_angle = glm::degrees(vector_angle_signed(to2d_xz(pCam->Front), normal_vec)) - 180;
   camera_change_direction(pCam, turn_angle, 0.f);
   CL_snap_player(player, normal_vec, penetration);

   player->player_state          = PLAYER_STATE_GRABBING;
   player->grabbing_entity       = entity;
   player->entity_ptr->velocity  = vec3(0);
   // after we are able to move while grabbing the ledge, this should move away from here
   {
      player->anim_final_dir        = rev_normal;
      player->anim_final_pos        = final_position;
      player->anim_orig_pos         = player->entity_ptr->position;
      player->anim_orig_dir         = normalize(to_xz(pCam->Front));
      player->entity_ptr->velocity  = vec3(0);
   }
}

// DONE
void P_state_change_grabbing_to_vaulting(Player* player)
{
   player->player_state          = PLAYER_STATE_VAULTING;
   player->anim_state            = P_ANIM_VAULTING;
   player->vaulting_entity_ptr   = player->grabbing_entity;
   player->grabbing_entity       = NULL;
}

// @TODO REFACTOR - 2d normal to 3D
void P_state_change_standing_to_vaulting(Player* player, Entity* entity, vec2 normal_vec, vec3 final_position)
{
   vec3 rev_normal = rev_2Dnormal(normal_vec);

   player->player_state             = PLAYER_STATE_VAULTING;
   player->anim_state               = P_ANIM_VAULTING;
   player->anim_final_pos           = final_position;
   player->anim_orig_pos            = player->entity_ptr->position;
   player->entity_ptr->velocity     = vec3(0);
   player->anim_orig_dir            = nrmlz(to_xz(pCam->Front));
   player->anim_final_dir           = rev_normal;
   player->vaulting_entity_ptr      = entity;
}

// DONE
void P_state_change_vaulting_to_standing(Player* player)
{
   G_INPUT_INFO.forget_last_mouse_coords  = true;
   G_INPUT_INFO.block_mouse_move          = false;
   player->player_state                   = PLAYER_STATE_STANDING;
   player->standing_entity_ptr            = player->vaulting_entity_ptr;
   player->vaulting_entity_ptr            = NULL;
   player->anim_finished_turning          = false;
}