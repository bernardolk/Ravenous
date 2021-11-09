
/* 
   --------------------
   > PLAYER STATE CODE
   --------------------
   Work in progress state machine-like modelling of player state 
*/

void P_state_change_jumping_to_falling(Player* player);
void P_state_change_standing_to_falling(Player* player);
void P_state_change_falling_to_standing(Player* player);
void P_state_change_standing_to_jumping(Player* player);


void P_change_state(Player* player, PlayerStateEnum new_state)
{
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
   }
}


void P_state_change_jumping_to_falling(Player* player)
{
   player->player_state           = PLAYER_STATE_FALLING;
   player->entity_ptr->velocity.y = 0;
   player->jumping_upwards          = false;

   // TEMP - DELETE LATER
   player->skip_collision_with_floor = NULL;
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
