void GP_update_player_state(Player* &player, WorldStruct* world)
{
   Entity* &player_entity = player->entity_ptr;

   switch(player->player_state)
   {

      case PLAYER_STATE_FALLING:
      {
         // // test collision with every object in scene entities vector
         // CL_run_collision_checks_falling(player);

         // if(player->action)
         //    GP_check_player_grabbed_ledge(player);
         break;
      }

      case PLAYER_STATE_STANDING:
      {
         assert(player->standing_entity_ptr != NULL);

         // step 1: if player switched floors, either just change his ground or make him slide if applicable
         if(player->walking)
            GP_check_for_floor_transitions_while_walking(player);
         else
            GP_check_for_floor_transitions(player);

         // step 2: check if player is actually sliding
         if(player->standing_entity_ptr->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
         {
            bool is_sliding                  = GP_check_for_sliding_slope_floor(player);
            if(is_sliding) break;
         }

         // step 2: position player at terrain's height
         auto terrain                        = CL_get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         player->entity_ptr->position.y      = terrain.overlap + player->half_height;

         // step 3: resolve possible collisions then do step 2 again
         // check for collisions with scene BUT with floor

         
         // CL_run_collision_checks_standing(player);

         terrain                             = CL_get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         player->entity_ptr->position.y      = terrain.overlap + player->half_height;

         // step 4: check if player is still standing on terrain
         auto xz_check                       = CL_get_horizontal_overlap_with_player(player->standing_entity_ptr, player);

         if(!xz_check.is_collided)
         {
            P_change_state(player, PLAYER_STATE_FALLING);
         }

         // if(player->free_running)
         //    GP_check_player_vaulting(player);

         break;
      }

      case PLAYER_STATE_JUMPING:
      {
         if (player->entity_ptr->velocity.y <= 0)
            P_change_state(player, PLAYER_STATE_FALLING);

         // test collision with every object in scene entities vector
         // CL_run_collision_checks_falling(player);

         // if(player->action)
         //    GP_check_player_grabbed_ledge(player);

         break;
      }

      case PLAYER_STATE_SLIDING:
      {
         assert(glm::length(player_entity->velocity) > 0);

         auto terrain = CL_check_for_floor_below_player(player);
         
         if(terrain.hit && terrain.entity != player->standing_entity_ptr)
         {
            cout << "EVICTED FROM SLOPE (" << player->standing_entity_ptr->name << ") to (" << terrain.entity->name << ")\n";
            player->player_state             = PLAYER_STATE_EVICTED_FROM_SLOPE;
            player->slope_player_was_ptr     = player->standing_entity_ptr;
            player->standing_entity_ptr      = terrain.entity;
            player->entity_ptr->velocity.y   = 0;
            player->entity_ptr->position.y   += terrain.distance;
            auto terrain_collision           = CL_get_terrain_height_at_player(player_entity, player->standing_entity_ptr);

            break;
         }

         // check for collisions with scene BUT with floor
         CL_run_collision_checks_standing(player);

         // if player is still standing after collision resolutions, correct player height, else, make fall
         auto terrain_collision = CL_get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         if(terrain_collision.is_collided)
         {
            player->entity_ptr->position.y   = terrain_collision.overlap + player->half_height;
         }
         else
         {
            // make player "slide" towards edge and fall away from floor
            std::cout << "PLAYER FELL (EVICTED)" << "\n";
            player->slope_player_was_ptr     = player->standing_entity_ptr;
            player->standing_entity_ptr      = NULL;
            player->player_state             = PLAYER_STATE_EVICTED_FROM_SLOPE;
            player->height_before_fall       = player_entity->position.y;
         }
         break;
      }

      case PLAYER_STATE_SLIDE_FALLING:
      {
         assert(glm::length(player_entity->velocity) > 0);

         auto terrain = CL_check_for_floor_below_player(player);
         
         if(terrain.hit && terrain.entity != player->standing_entity_ptr)
         {
            cout << "EVICTED FROM SLOPE (" << player->standing_entity_ptr->name << ") to (" << terrain.entity->name << ")\n";
            player->player_state             = PLAYER_STATE_EVICTED_FROM_SLOPE;
            player->slope_player_was_ptr     = player->standing_entity_ptr;
            player->standing_entity_ptr      = terrain.entity;
            player->entity_ptr->velocity.y   = 0;
            player->entity_ptr->position.y   += terrain.distance;

            break;
         }

         // ... or player fell out of slope
         auto terrain_collision              = CL_get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         if(terrain_collision.is_collided)
         {
            player->entity_ptr->position.y   = terrain_collision.overlap + player->half_height;
         }  
         else
         {
            // make player "slide" towards edge and fall away from floor
            std::cout << "PLAYER FELL (EVICTED)" << "\n";
            player->slope_player_was_ptr     = player->standing_entity_ptr;
            player->standing_entity_ptr      = NULL;
            player->player_state             = PLAYER_STATE_EVICTED_FROM_SLOPE;
            player->height_before_fall       = player_entity->position.y;
         }
         
         break;
      }

      case PLAYER_STATE_EVICTED_FROM_SLOPE:
      {
         // here, player can already be considered standing somewhere or not. Which is weird.
         assert(player->slope_player_was_ptr != NULL);

         // Here it is assumed player ALREADY has a velocity vec pushing him away from the slope
         assert(glm::length(player_entity->velocity) > 0);

         // check if still colliding with floor, if so, let player keep sliding, if not, change to FALLING
         auto c_test = CL_get_horizontal_overlap_with_player(player->slope_player_was_ptr, player);
         if(!c_test.is_collided)
         {
            player->slope_player_was_ptr        = NULL;

            if(player->standing_entity_ptr != NULL)
               player->player_state             = PLAYER_STATE_STANDING;
            else
            {
               player->player_state             = PLAYER_STATE_FALLING;
               player->entity_ptr->velocity.x   = 0;
               player->entity_ptr->velocity.z   = 0;
            }
         }
         break;
      }

      case PLAYER_STATE_GRABBING:
      {
         if(!player->action)
         {
            player->player_state                = PLAYER_STATE_FALLING;
            player->grabbing_entity             = NULL;
         }

         break;
      }
   }
}