void GP_update_player_state_OLD(Player* &player, WorldStruct* world)
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
            player->slope_player_was_ptr = NULL;

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

float PLAYER_STEPOVER_LIMIT   = 0.1;
float PLAYER_STEPOVER_DELTA  = 0.03;


//@todo - Rethink the name and purpose of this function
RaycastTest CL_do_c_vtrace(Player* player)
{
   auto downward_ray    = Ray{player->feet() + vec3{0.0f, PLAYER_STEPOVER_LIMIT, 0.0f}, -UNIT_Y};
   RaycastTest raytest  = test_ray_against_scene(downward_ray, player->entity_ptr->id);

   if(!raytest.hit) return RaycastTest{false};

    // draw arrow
   auto hitpoint = point_from_detection(downward_ray, raytest);
   IM_RENDER.add_line(IMHASH, hitpoint, hitpoint + UNIT_Y * 3.f, 1.0, true, COLOR_GREEN_1);
   IM_RENDER.add_point(IMHASH, hitpoint, 1.0, true, COLOR_GREEN_3);

   if(abs(raytest.distance - PLAYER_STEPOVER_LIMIT) <= PLAYER_STEPOVER_LIMIT)
      return RaycastTest{true, raytest.distance - PLAYER_STEPOVER_LIMIT, raytest.entity};
   else
      return RaycastTest{false};
}


void GP_update_player_state(Player* &player, WorldStruct* world)
{
   // UPDATE COLLISION BUFFERS 
   {
      CL_update_player_world_cells(player);
      /*@todo: unless this becomes a performance problem, its easier to recompute the buffer every frame
               then to try placing this call everytime necessary */
      CL_recompute_collision_buffer_entities(player);
   }
   
   // compute player next position
   auto next_position = GP_player_next_position(player);


   CL_Ignore_Colliders.empty();

   switch(player->player_state)
   {
      case PLAYER_STATE_STANDING:
      {

         /* NOTES
          1. If we move towards a steppable collider with an angle where the cylinder hits
             it at a different point than the movement direction, the fwd raycast wont it the collider
             and GJK will push player away from it, turning it into an obstacle.
             I like the idea of having *some* resistance from a step when moving from an angle, making
             it not completely transparent to the player when moving around, but at the same time,
             the angles allowed for stepping need to be more than a single one.
             I am thinking on using a circular sector (internal angle, angular range, whatever)
             to do multiple raycasts for this test, both forwards and backwards.
             I can then tweak it to achieve the best resistance / movement fluidity tradeoff.
             I am unsure at the moment if we need to do that both to fwd and bwd traces or just for fwd.
         */

         // RAYCAST FORWARD TO DISABLE COLLISION IF FLOOR DETECTED
         auto p_next_pos_fwd_periphery = next_position + player->v_dir_historic * player->radius;
         auto f_vtrace_pos = vec3(p_next_pos_fwd_periphery.x, player->top().y + 1, p_next_pos_fwd_periphery.z);

         auto v_trace_fwd_ray = Ray{f_vtrace_pos, -UNIT_Y};
         RaycastTest fwd_vtrace = test_ray_against_scene(v_trace_fwd_ray);
         if(fwd_vtrace.hit)
         {
            auto hitpoint = point_from_detection(v_trace_fwd_ray, fwd_vtrace);
            // draw arrow
            IM_RENDER.add_line(IMHASH, hitpoint, hitpoint + UNIT_Y * 3.f, COLOR_RED_1);
            IM_RENDER.add_point(IMHASH, hitpoint, COLOR_RED_3);

            float delta_y = abs(player->feet().y - hitpoint.y);
            if(delta_y <= PLAYER_STEPOVER_LIMIT)
            {
               // disable collision with potential floor / terrain
               CL_Ignore_Colliders.add(fwd_vtrace.entity);
            }
         }

         // RAYCAST BACKWARD TO DISABLE COLLISION IF FLOOR DETECTED
         auto p_pos_bwd_periphery = player->entity_ptr->position - player->v_dir_historic * player->radius;
         auto b_vtrace_pos = vec3(p_pos_bwd_periphery.x, player->top().y + 1, p_pos_bwd_periphery.z);

         auto v_trace_bwd_ray = Ray{b_vtrace_pos, -UNIT_Y};
         RaycastTest bwd_vtrace = test_ray_against_scene(v_trace_bwd_ray);
         if(bwd_vtrace.hit)
         {
            auto hitpoint = point_from_detection(v_trace_bwd_ray, bwd_vtrace);
            // draw arrow
            IM_RENDER.add_line(IMHASH, hitpoint, hitpoint + UNIT_Y * 3.f, COLOR_YELLOW_1);
            IM_RENDER.add_point(IMHASH, hitpoint, COLOR_YELLOW_3);

            float delta_y = abs(player->feet().y - hitpoint.y);
            if(delta_y <= PLAYER_STEPOVER_LIMIT)
            {
               // disable collision with current / previous floor
               CL_Ignore_Colliders.add(bwd_vtrace.entity);
            }
         }

         // MOVE PLAYER FORWARD
         player->entity_ptr->position = next_position;

         // DO PLAYER CENTER VTRACE IN NEXT POSITION
         auto c_vtrace = CL_do_c_vtrace(player);

         // UPDATE PLAYER HEIGHT
         if(c_vtrace.hit)
         {
            // if we are here, it means we have hit something in the 'stepover' region.
            // very confusing :/
            // thats why I want to refactor the CL_do_c_vtrace procedure to something else

            player->entity_ptr->position.y -= c_vtrace.distance;     // positive distance is downwards
            CL_Ignore_Colliders.add(c_vtrace.entity);
         }
         else
         {
            // 1. compute direction of fall using player's momentum
            // 2. compute terminal position after sliding player towards the fall
               // ( maybe by testing collision repeatedly until no collision, with a stop point)
            // 3. if player cant fall, DO NOT update players height (he will kinda float above the hole)
            // 4. if he can fall, then do P_change_state(player, PLAYER_STATE_FALLING);
         }

         CL_run_iterative_collision_detection(player);

         break;
      }
      case PLAYER_STATE_FALLING:
      {
         break;
      }
      case PLAYER_STATE_JUMPING:
      {
         break;
      }
   }
   
}