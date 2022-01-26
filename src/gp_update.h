
void GP_update_player_state                     (Player* &player);
vec3 GP_player_standing_get_next_position  (Player* player);
void GP_check_trigger_interaction               (Player* player);
void GP_check_player_grabbed_ledge              (Player* player);
bool GP_check_player_vaulting                   (Player* player);
bool GP_simulate_player_collision_in_falling_trajectory(Player* player, vec2 xz_velocity);
// bool GP_scan_for_terrain(vec3 center, float radius, vec2 orientation0, float angle, int subdivisions);
// bool GP_do_vtrace_for_terrain(vec3 vtrace_origin, float terrain_baseline_height, vec3 debug_color);

float SLOPE_MIN_ANGLE = 0.4;


void GP_update_player_state(Player* &player)
{   
   switch(player->player_state)
   {
      case PLAYER_STATE_STANDING:
      {
         // compute player next position
         auto next_position = GP_player_standing_get_next_position(player);

         // move player forward
         player->entity_ptr->position = next_position;
         player->update();

         vec3 player_btm_sphere_center = player->entity_ptr->position + vec3(0, player->radius, 0);
         vec3 contact_point =  player_btm_sphere_center + -player->last_terrain_contact_normal * player->radius;
         IM_RENDER.add_line(IMHASH, player_btm_sphere_center, contact_point, COLOR_YELLOW_1);

         /* Current system: Here we are looping at most twice on the:
            "Do stepover Vtrace, Adjust player's position to terrain, check collisions" loop
            so we can detect when we try stepping up/down into a place where the player can't
            fit in.
         */

         int it = 0;
         while(it < 2)
         {
            it++;
            auto vtrace = CL_do_stepover_vtrace(player);

            // snap player to the last terrain contact point detected if its a valid stepover hit
            if(vtrace.hit && (vtrace.delta_y > 0.0004 || vtrace.delta_y < 0))
            {
               player->entity_ptr->position.y -= vtrace.delta_y;
               player->update();
            }
            
            // resolve collisions
            auto results = CL_test_and_resolve_collisions(player);

            // iterate on collision results
            bool collided_with_terrain = false;
            CL_Results slope;
            for (int i = 0; i < results.count; i ++)
            {
               auto result = results.results[i];

               collided_with_terrain = dot(result.normal, UNIT_Y) > 0;

               if(collided_with_terrain)
                  player->last_terrain_contact_normal = result.normal;

               bool collided_with_slope = dot(result.normal, UNIT_Y) >= SLOPE_MIN_ANGLE;
               if(collided_with_slope && result.entity->slidable)
                  slope = result;
            }

            // if floor is no longer beneath player's feet
            if (!vtrace.hit)
            {
               /* Here we do a simulation to check if player would fit going through the abyss.
                  If he doesn't fit, then ignore the hole and let player walk through it. */
               
               //@todo: SLOW

               // first pass test to see if player fits in hole
               auto p_pos0 = player->entity_ptr->position;
               player->entity_ptr->position += player->v_dir_historic * player->radius;
               player->update();
               bool collided = CL_run_tests_for_fall_simulation(player);

               player->entity_ptr->position = p_pos0;
               player->update();

               if(!collided)
               {
                  /* do a complete simulation of the fall (maybe unncessary... ) */
                  // give player a push if necessary
                  float fall_momentum_intensity = player->speed;
                  if(fall_momentum_intensity < player->fall_from_edge_push_speed)
                     fall_momentum_intensity = player->fall_from_edge_push_speed;


                  vec2 fall_momentum_dir;
                  fall_momentum_dir = to2d_xz(player->v_dir_historic);
                  vec2 fall_momentum = fall_momentum_dir * fall_momentum_intensity;

                  bool can_fall = GP_simulate_player_collision_in_falling_trajectory(player, fall_momentum);

                  if(can_fall)
                  {
                     player->entity_ptr->velocity = to3d_xz(fall_momentum);
                     GP_change_player_state(player, PLAYER_STATE_FALLING);
                     player->update();
                     break;
                  }
                  else
                     editor_print("Player won't fit if he falls here.", 1000);
               }
               else
                     editor_print("We could fall but we are smarts", 1000);

               break;
            }

            // collided with nothing or with terrain only, break
            else if(results.count == 0 || (collided_with_terrain && results.count == 1))
               break;

            else if(slope.collision)
            {
               PlayerStateChangeArgs args;
               args.normal = slope.normal;
               GP_change_player_state(player, PLAYER_STATE_SLIDING, args);
               break;
            }
         }

         // Check interactions
         if(player->want_to_grab){
            GP_check_player_grabbed_ledge(player);
            editor_persist_print("Ran check player grabbed ledge", 1000);
         }

         break;
      }


      case PLAYER_STATE_FALLING:
      {
         player->entity_ptr->velocity += G_FRAME_INFO.duration * player->gravity; 
         player->entity_ptr->position += player->entity_ptr->velocity * G_FRAME_INFO.duration;
         player->update();

         auto results = CL_test_and_resolve_collisions(player);
         for (int i = 0; i < results.count; i ++)
         {
            auto result = results.results[i];

            // slope collision
            {
               bool collided_with_slope = dot(result.normal, UNIT_Y) >= SLOPE_MIN_ANGLE;
               if(collided_with_slope && result.entity->slidable)
               {
                  PlayerStateChangeArgs args;
                  args.normal = result.normal;
                  GP_change_player_state(player, PLAYER_STATE_SLIDING, args);
                  return;
               }
            }

            // floor collision
            {
               bool collided_with_terrain = dot(result.normal, UNIT_Y) > 0;
               if(collided_with_terrain)
               {
                  GP_change_player_state(player, PLAYER_STATE_STANDING);
                  return;
               }
            }

            // else
            {
               CL_wall_slide_player(player, result.normal);
            }
         }

         break;
      }


      case PLAYER_STATE_JUMPING:
      {
         auto& v = player->entity_ptr->velocity;

         bool no_move_command = player->v_dir.x == 0 && player->v_dir.z == 0;
         if(player->jumping_upwards && !no_move_command)
         {
            if(player->speed < player->air_speed)
            {
               player->speed += player->air_delta_speed;
               v += player->v_dir * player->air_delta_speed;
            }
         }

         v += G_FRAME_INFO.duration * player->gravity; 
         player->entity_ptr->position += player->entity_ptr->velocity * G_FRAME_INFO.duration;
         player->update();

         auto results = CL_test_and_resolve_collisions(player);
         for (int i = 0; i < results.count; i ++)
         {
            auto result = results.results[i];

            // collision with terrain while jumping should be super rare I guess ...
            // slope collision
            {
               bool collided_with_slope = dot(result.normal, UNIT_Y) >= SLOPE_MIN_ANGLE;
               if(collided_with_slope && result.entity->slidable)
               {
                  PlayerStateChangeArgs args;
                  args.normal = result.normal;
                  GP_change_player_state(player, PLAYER_STATE_SLIDING, args);
                  return;
               }
            }

            // floor collision 
            {
               bool collided_with_terrain = dot(result.normal, UNIT_Y) > 0;
               if(collided_with_terrain)
               {
                  GP_change_player_state(player, PLAYER_STATE_STANDING);
                  return;
               }
            }

            // else
            {
               CL_wall_slide_player(player, result.normal);
            }
         }

         // @todo - need to include case that player touches inclined terrain
         //          in that case it should also stand (or fall from ledge) and not
         //          directly fall.
         if(results.count > 0)
            GP_change_player_state(player, PLAYER_STATE_FALLING);

         else if (player->entity_ptr->velocity.y <= 0)
            GP_change_player_state(player, PLAYER_STATE_FALLING);

         break;
      }


      case PLAYER_STATE_SLIDING:
      {
         IM_RENDER.add_line(IMHASH, player->entity_ptr->position, player->entity_ptr->position + 1.f * player->sliding_direction, COLOR_RED_2);

         player->entity_ptr->velocity = player->v_dir * player->slide_speed;

         player->entity_ptr->position += player->entity_ptr->velocity * G_FRAME_INFO.duration;
         player->update();


         // RESOLVE COLLISIONS AND CHECK FOR TERRAIN CONTACT
         auto results = CL_test_and_resolve_collisions(player);

         bool collided_with_terrain = false;
         for (int i = 0; i < results.count; i ++)
         {
            // iterate on collision results
            auto result = results.results[i];
            collided_with_terrain = dot(result.normal, UNIT_Y) > 0;
            if(collided_with_terrain)
               player->last_terrain_contact_normal = result.normal;
         }

         if(collided_with_terrain)
         {
            GP_change_player_state(player, PLAYER_STATE_STANDING);
            break;
         }

         auto vtrace = CL_do_stepover_vtrace(player);
         if(!vtrace.hit)
         {
            GP_change_player_state(player, PLAYER_STATE_FALLING);
         }

         break;
      }
   }
   
}


// bool GP_scan_for_terrain(vec3 center, float radius, vec2 orientation0, float angle, int subdivisions)
// {
//    /* Does a circular array of raycasts according to parameters.
//       The circle will be considered to be 'touching the ground', hence limits for stepping up or down are applied
//       from the "center" arg y component.
  
//       center: circle's center
//       radius: circle radius
//       orientation0: reference direction for first ray
//       angle: angle span from 0 to 360
//       subdivisions: controls how many rays to shoot
//    */

//       bool hit_terrain = false;
//       vec3 orientation = normalize(to3d_xz(orientation0));
//       float delta_angle = angle / subdivisions;
//       float current_angle = 0;
//       while(current_angle <= angle)
//       {
//          orientation = rotate(orientation, glm::radians(delta_angle), UNIT_Y);
//          vec3 ray_origin = center + orientation * radius;
//          // moves ray up a bit
//          ray_origin.y = center.y + 1;

//          vec3 color = COLOR_RED_1;
//          if(current_angle <= angle / 2)
//             color = COLOR_RED_2;

//          bool hit = GP_do_vtrace_for_terrain(ray_origin, center.y, color);
//          hit_terrain = hit_terrain || hit;

//          current_angle += delta_angle;
//       }

//       return hit_terrain;
// }


// bool GP_do_vtrace_for_terrain(vec3 vtrace_origin, float terrain_baseline_height, vec3 debug_color = COLOR_RED_1)
// {
//    auto vtrace_ray = Ray{ vtrace_origin, -UNIT_Y };
//    RaycastTest vtrace = test_ray_against_scene(vtrace_ray);
//    if(vtrace.hit)
//    {
//       auto hitpoint = point_from_detection(vtrace_ray, vtrace);
//       // draw arrow
//       IM_RENDER.add_line (IMCUSTOMHASH(to_string(vtrace_origin)), hitpoint, vtrace_origin, debug_color);
//       IM_RENDER.add_point(IMCUSTOMHASH(to_string(vtrace_origin)), hitpoint, debug_color);

//       float delta_y = abs(terrain_baseline_height - hitpoint.y);
//       if(delta_y <= PLAYER_STEPOVER_LIMIT)
//       {
//          // disable collision with potential floor / terrain
//          CL_Ignore_Colliders.add(vtrace.entity);
//          return true;
//       }
//    }

//    return false;
// }


vec3 GP_player_standing_get_next_position(Player* player)
{
   // updates player position
   auto& v           = player->entity_ptr->velocity;
   auto& v_dir       = player->v_dir;
   auto& state       = player->player_state;

   bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

   auto dt = G_FRAME_INFO.duration;

   if(v_dir.x != 0 || v_dir.y != 0 || v_dir.z != 0)
      player->v_dir_historic = v_dir;
   else if(player->v_dir_historic == vec3(0))
      player->v_dir_historic = normalize(to_xz(G_SCENE_INFO.views[FPS_CAM]->Front));

   if(player->speed < 0.f || no_move_command)
      player->speed = 0;

   auto& speed    = player->speed;
   float d_speed  = player->acceleration * dt;


   float speed_limit;
   if(player->dashing)
      speed_limit = player->dash_speed;
   else if(player->walking)
      speed_limit = player->walk_speed;
   else
      speed_limit = player->run_speed;
   
   bool stopped = (speed > 0 && no_move_command);
   if(stopped)
   { 
      speed = 0;
      d_speed = 0;
   }

   speed += d_speed;

   if(speed > speed_limit)
      speed = speed_limit;

   v = speed * v_dir;     // if no movement command is issued, v_dir = 0,0,

   return player->entity_ptr->position + v * dt;
}


// -------------------
// > ACTION
// -------------------

void GP_check_trigger_interaction(Player* player)
{
   auto interactables = G_SCENE_INFO.active_scene->interactables;
   For(interactables.size())
   {
      auto interactable = interactables[i];

      //@todo: do a cylinder vs cylinder or cylinder vs aabb test here
      Mesh trigger_collider = interactable->get_trigger_collider();
      GJK_Result gjk_test = CL_run_GJK(&player->entity_ptr->collider, &trigger_collider);
      if(gjk_test.collision)
      {
         editor_print("Trigger Interaction", 1000);

         switch(interactable->type)
         {
            case EntityType_Checkpoint:
            {
               player->set_checkpoint(interactable);
               break;
            }
            case EntityType_TimerTrigger:
            {
               Game_State.start_timer(interactable);
               break;
            }
         }
      }
   }
}

// -------------------
// > LEDGE GRABBING
// -------------------
void GP_check_player_grabbed_ledge(Player* player)
{
   Ledge ledge = CL_perform_ledge_detection(player);
   if(ledge.empty)
      return;
   vec3 position = CL_get_final_position_ledge_vaulting(player, ledge);

   PlayerStateChangeArgs args;
   args.ledge = ledge;
   args.final_position = position;
   GP_change_player_state(player, PLAYER_STATE_VAULTING, args);
}

// void GP_check_player_grabbed_ledge(Player* player)
// {
//    // ledge grab y tollerance
//    const float y_tol = 0.1;
//    // half the ledge grab semicircle region angle, in degrees 
//    const float s_theta = 40;
//    // radius of detection
//    const float dr = 0.1;

//    float player_y = player->top().y;
//    auto camera_f = vec2(pCam->Front.x, pCam->Front.z);


//    for(int i = 0; i < G_BUFFERS.entity_buffer->size; i++)
//    {
//       Entity* entity = G_BUFFERS.entity_buffer->buffer[i].entity;

//       if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
//       {
//          float edge_y = entity->position.y + entity->get_height();
//          if(!(player_y < edge_y + y_tol && player_y > edge_y - y_tol))
//             continue;

//          auto [x0, x1, z0, z1] = entity->get_rect_bounds();
//          auto test = CL_circle_vs_square(
//             player->entity_ptr->position.x, 
//             player->entity_ptr->position.z, 
//             player->radius + dr,
//             x0, x1, z0, z1
//          );

//          if(!test.is_collided)
//             continue;

//          float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
//          float min_theta = 180 - s_theta;
//          float max_theta = 180 + s_theta;
//          if(min_theta <= theta && theta <= max_theta)
//          {
//             // checks if area above ledge is free for standing
//             vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
//             IM_RENDER.add_mesh(IMHASH, player->entity_ptr, future_pos);
//             if(CL_test_in_mock_position(player, future_pos))
//                continue;

//             PlayerStateChangeArgs ps_args;
//             ps_args.entity = entity;
//             // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
//             ps_args.final_position = future_pos;
//             ps_args.penetration = dr - test.overlap;

//             GP_change_player_state(player, PLAYER_STATE_GRABBING, ps_args);
//             return;
//          }
//       }

//       else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
//       {
//          float edge_y = entity->position.y;
//          if(!(player_y < edge_y + y_tol && player_y > edge_y - y_tol))
//             continue;

//          auto [x0, x1, z0, z1] = entity->get_rect_bounds();
//          auto test = CL_circle_vs_square(
//             player->entity_ptr->position.x, 
//             player->entity_ptr->position.z, 
//             player->radius + dr,
//             x0, x1, z0, z1
//          );

//          if(!test.is_collided)
//             continue;

//          // player is not facing slope's inclined face
//          if(get_slope_normal(entity) != test.normal_vec)
//             continue;

//          float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
//          float min_theta = 180 - s_theta;
//          float max_theta = 180 + s_theta;
//          if(min_theta <= theta && theta <= max_theta)
//          {
//             // checks if area above ledge is free for standing
//             vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
//             IM_RENDER.add_mesh(IMHASH, player->entity_ptr, future_pos);
//             if(CL_test_in_mock_position(player, future_pos, entity))
//                continue;

//             PlayerStateChangeArgs ps_args;
//             ps_args.entity = entity;
//             // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
//             ps_args.final_position = future_pos;
//             ps_args.penetration = dr - test.overlap;

//             GP_change_player_state(player, PLAYER_STATE_GRABBING, ps_args);
//             return;
//          }
//       }
//    }
// }


// -------------------
// > VAULTING
// -------------------

// bool GP_check_player_vaulting(Player* player)
// {
//    // action cone half theta 
//    const float s_theta = 40;
//    // radius of detection
//    const float dr = 0.1;

//    float player_y = player->entity_ptr->position.y;
//    auto camera_f = vec2(pCam->Front.x, pCam->Front.z);

//    for(int i = 0; i < G_BUFFERS.entity_buffer->size; i++)
//    {
//       Entity* entity = G_BUFFERS.entity_buffer->buffer[i].entity;

//       if(entity->collision_geometry_type != COLLISION_ALIGNED_BOX)
//          continue;

//       float rel_height = (entity->position.y + entity->get_height()) - player->feet().y;

//       // short platforms should be ignored since we will use navigation meshes that include them smoothly with a nav ramp 
//       // and therefore going over them do not count as 'vaulting moves'
//       if(rel_height < 0.3) // also makes sure we only get positive rel heights
//          continue;
      
//       if(rel_height >= player->half_height * 2)
//          continue;

//       auto [x0, x1, z0, z1] = entity->get_rect_bounds();
//       auto test = CL_circle_vs_square(
//          player->entity_ptr->position.x,
//          player->entity_ptr->position.z,
//          player->radius + dr,
//          x0, x1, z0, z1
//       );

//       if(!test.is_collided)
//          continue;

//       float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
//       float min_theta = 180 - s_theta;
//       float max_theta = 180 + s_theta;
//       if(min_theta <= theta && theta <= max_theta)
//       {
//          // checks if area above ledge is free for standing
//          vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
//          // IM_RENDER.add_mesh(IMHASH, player->entity_ptr, future_pos);
//          if(CL_test_in_mock_position(player, future_pos))
//          {
//             editor_print("Vaulting failed.");
//             continue;
//          }
         
//          PlayerStateChangeArgs ps_args;
//          ps_args.entity = entity;
//          // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
//          ps_args.final_position = future_pos;
//          ps_args.penetration = dr - test.overlap;

//          GP_change_player_state(player, PLAYER_STATE_VAULTING, ps_args);
//          return true;
//       }
//    }
//    return false;
// }


// void GP_check_player_events(Player* player)
// {
//    // Player death
//    if(player->lives <= 0)
//    {
//       G_BUFFERS.rm_buffer->add("PLAYER DIED (height:" + format_float_tostr(player->fall_height_log, 2) + " m)", 3000);
//       player->die();
//       return;
//    }
// }