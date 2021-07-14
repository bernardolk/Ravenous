void update_player_state(Player* &player, WorldStruct* world);
void move_player(Player* player);


// ---------------------
// UPDATE PLAYER STATE
// --------------------- 
void update_player_state(Player* &player, WorldStruct* world)
{
   Entity* &player_entity = player->entity_ptr;

   if(player->lives <= 0)
   {
      G_BUFFERS.rm_buffer->add("PLAYER DIED (height:" + format_float_tostr(player->fall_height_log, 2) + " m)", 3000);
      player->die();
      return;
   }

   switch(player->player_state)
   {
      case PLAYER_STATE_FALLING:
      {
         // test collision with every object in scene entities vector
         run_collision_checks_falling(player);
         break;
      }
      case PLAYER_STATE_STANDING:
      {
         // step 1: if player switched floors, either just change his ground or make him slide if applicable
         check_for_floor_transitions(player);

         // step 2: position player at terrain's height
         auto terrain = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         player->entity_ptr->position.y = terrain.overlap + player->half_height;

         // step 3: resolve possible collisions then do step 2 again
         // check for collisions with scene BUT with floor
         run_collision_checks_standing(player);

         terrain = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         player->entity_ptr->position.y = terrain.overlap + player->half_height;

         // step 4: check if player is still standing on terrain
         auto xz_check = get_horizontal_overlap_with_player(player->standing_entity_ptr, player);

         if(!xz_check.is_collided)
         {
            std::cout << "PLAYER FELL" << "\n";
            player_entity->velocity.y = - 1 * player->fall_speed;
            player_entity->velocity.x *= 0.5;
            player_entity->velocity.z *= 0.5;

            player->player_state = PLAYER_STATE_FALLING;
            player->height_before_fall = player_entity->position.y;
         }
         break;
      }
      case PLAYER_STATE_JUMPING:
      {
         /* remarks about the jump system:
            we set at input key_press time (input.h) a high velocity upward for the player
            at each frame we decrement a little bit from the y velocity component using delta frame time
            IDEALLY we would set our target jump height and let the math work itself out from there.
            For our prototype this should be fine.
         */
         if (player->entity_ptr->velocity.y <= 0)
         {
            player->jumping_upwards = false;
            player->entity_ptr->velocity.y = 0;
            player->player_state = PLAYER_STATE_FALLING;
         }

         // test collision with every object in scene entities vector
         run_collision_checks_falling(player);
         break;
      }
      case PLAYER_STATE_SLIDING:
      {
         assert(glm::length(player_entity->velocity) > 0);

         auto terrain = check_for_floor_below_player(player);
         
         if(terrain.hit && terrain.entity != player->standing_entity_ptr)
         {
            cout << "EVICTED FROM SLOPE (" << player->standing_entity_ptr->name << ") to (" << terrain.entity->name << ")\n";
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = terrain.entity;
            player->entity_ptr->velocity.y = 0;
            player->entity_ptr->position.y += terrain.distance;
            auto terrain_collision = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
            break;
         }

         // check for collisions with scene BUT with floor
         run_collision_checks_standing(player);

         // if player is still standing after collision resolutions, correct player height, else, make fall
         auto terrain_collision = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         if(terrain_collision.is_collided)
         {
            player->entity_ptr->position.y = terrain_collision.overlap + player->half_height;
         }
         else
         {
            // make player "slide" towards edge and fall away from floor
            std::cout << "PLAYER FELL (EVICTED)" << "\n";
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = NULL;
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
            player->height_before_fall = player_entity->position.y;
         }
         break;
      }
      case PLAYER_STATE_SLIDE_FALLING:
      {
         assert(glm::length(player_entity->velocity) > 0);

         auto terrain = check_for_floor_below_player(player);
         
         if(terrain.hit && terrain.entity != player->standing_entity_ptr)
         {
            cout << "EVICTED FROM SLOPE (" << player->standing_entity_ptr->name << ") to (" << terrain.entity->name << ")\n";
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = terrain.entity;
            player->entity_ptr->velocity.y = 0;
            player->entity_ptr->position.y += terrain.distance;
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
            break;
         }

         // ... or player fell out of slope
         auto terrain_collision = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         if(terrain_collision.is_collided)
         {
            player->entity_ptr->position.y = terrain_collision.overlap + player->half_height;
         }  
         else
         {
            // make player "slide" towards edge and fall away from floor
            std::cout << "PLAYER FELL (EVICTED)" << "\n";
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = NULL;
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
            player->height_before_fall = player_entity->position.y;
         }
         
         break;
      }
      // case PLAYER_STATE_FALLING_FROM_EDGE:
      // {
      //    // Here it is assumed player ALREADY has a velocity vec pushing him away from the platform he is standing on
      //    assert(glm::length(player_entity->velocity) > 0);
      //    // check if still colliding with floor, if so, let player keep sliding, if not, change to FALLING
      //    Collision c_test;
      //    c_test = get_horizontal_overlap_with_player(player->standing_entity_ptr, player);
            
      //    if(!c_test.is_collided)
      //    {
      //       player->player_state = PLAYER_STATE_FALLING;
      //       player->standing_entity_ptr = NULL;
      //       // player_entity->velocity = vec3(0, 0, 0); 
      //    }
      //    break;
      // }
      case PLAYER_STATE_EVICTED_FROM_SLOPE:
      {
         // here, player can already be considered standing somewhere or not. Which is weird.
         assert(player->slope_player_was_ptr != NULL);
         // Here it is assumed player ALREADY has a velocity vec pushing him away from the slope
         assert(glm::length(player_entity->velocity) > 0);
         // check if still colliding with floor, if so, let player keep sliding, if not, change to FALLING
         auto c_test = get_horizontal_overlap_with_player(player->slope_player_was_ptr, player);
         if(!c_test.is_collided)
         {
            player->slope_player_was_ptr = NULL;
            if(player->standing_entity_ptr != NULL)
               player->player_state = PLAYER_STATE_STANDING;
            else
            {
               player->entity_ptr->velocity.x = 0;
               player->entity_ptr->velocity.z = 0;
               player->player_state = PLAYER_STATE_FALLING;
            }
         }
         break;
      }
   }
}

void move_player(Player* player)
{
   auto& v = player->entity_ptr->velocity;
   auto& v_dir = player->v_dir;
   auto& state = player->player_state;

   bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

   if(player->speed < 0.f || no_move_command)
      player->speed = 0;

   // string v_dir_string = "speed: " + to_string(player->speed);
   // G_BUFFERS.rm_buffer->add(v_dir_string, 0);

   auto dt = G_FRAME_INFO.duration * G_FRAME_INFO.time_step;

   switch(state)
   {
      case PLAYER_STATE_STANDING:
      {
         auto& speed = player->speed;
         float d_speed = player->acceleration * dt;

         // deacceleration
         bool contrary_movement = !comp_sign(v_dir.x, v.x) || !comp_sign(v_dir.z, v.z);
         bool stopped_dashing = !player->dashing && square_GT(v + d_speed, player->run_speed);

         if((speed > 0 && no_move_command) || stopped_dashing)
            d_speed *= -1;            
         else if(player->dashing && square_GE(v + d_speed, player->dash_speed))
            d_speed = 0;

         speed += d_speed;

         // if no movement command is issued, v_dir = 0,0,0 !
         v = speed * v_dir;    

         break;
      }
      case PLAYER_STATE_JUMPING:
      {
         // mid air controls
         if(player->jumping_upwards && !no_move_command)
         {
            v.x += v_dir.x * player->air_delta_speed;
            v.z += v_dir.z * player->air_delta_speed;
         }

         // dampen player y speed (vf = v0 - g*t)
         v.y -=  player->fall_acceleration * dt;
         break;
      }
      case PLAYER_STATE_FALLING:
      {
         // dampen player y speed (vf = v0 - g*t)
         v.y -= player->fall_acceleration * dt;
         break;
      }
      case PLAYER_STATE_SLIDING:
      {
         if(player->jumping_from_slope)
         {
            player->jumping_from_slope = false;
            player->player_state = PLAYER_STATE_JUMPING;
            player->height_before_fall = player->entity_ptr->position.y;
            player->entity_ptr->velocity = player->v_dir * player->slide_jump_speed;
         }
         break;
      }
   }

   // update player position
   player->prior_position = player->entity_ptr->position;
   player->entity_ptr->position += v * dt;
   return;
}