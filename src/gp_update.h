
void GP_update_player_state(Player* &player, WorldStruct* world);
vec3 GP_get_player_next_position_when_standing  (Player* player);
void GP_check_trigger_interaction               (Player* player);
bool GP_check_player_grabbed_ledge              (Player* player, Entity* entity);
bool GP_check_player_vaulting                   (Player* player);
RaycastTest CL_do_c_vtrace                      (Player* player);

float PLAYER_STEPOVER_LIMIT   = 0.21;

void GP_update_player_state(Player* &player, WorldStruct* world)
{   
   // compute player next position
   auto next_position = GP_get_player_next_position_when_standing(player);


   // Remove entities added to the 'ignored for collision test' list if they aren't colliding with player anymore
   for(int i = 0; i < CL_Ignore_Colliders.count; i++)
   {
      auto entity = CL_Ignore_Colliders.list[i];
      auto result = CL_run_GJK(&entity->collider, &player->entity_ptr->collider);

      if(!result.collision)
         CL_Ignore_Colliders.remove(entity);
   }

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
            /* If player was standing on something and we added the collider to the ignored colliders for collision,
               then, run a simulation, moving player like gravity would, testing for collision in each step and
               once we are not colliding with the terrain colliders anymore, check if player fits without colliding with
               anything in that position and then allow him to fall through.
               If he doesn't fit, then ignore the hole and let player walk through it.
            */

            if(CL_Ignore_Colliders.count > 0)
            {
               // configs
               float d_frame = 0.014;

               vec3 vel       = player->entity_ptr->velocity;
               auto pos_0     = player->entity_ptr->position;

               // Give player a 'push'
               if(abs(vel.x) < player->fall_from_edge_push_speed && abs(vel.z) < player->fall_from_edge_push_speed)
                  vel = player->v_dir_historic * player->fall_from_edge_push_speed;

               float max_iterations = 120;

               //IM_RENDER.add_point(IMHASH, player->entity_ptr->position, 2.0, false, COLOR_GREEN_1, 1);

               int iteration = 0;
               bool can_fall = true;
               while(true)
               {
                  vel += d_frame * player->gravity; 
                  player->entity_ptr->position += vel * d_frame;
                  //IM_RENDER.add_point(IM_ITERHASH(iteration), player->entity_ptr->position, 2.0, true, COLOR_GREEN_1, 1);

                  // @todo - Note: this will probably have a bug because we are not updating player's world cells
                  // so if player is going to fall from one world cell into another, we wouldn't test
                  // collisions for entities in the next world cell.
                  // So, for tests where we don't use current player position, we need to figure out
                  // how to consider the relevant entities, maybe test for entities in collision buffer
                  // plus entities in nearby world cells?
                  player->entity_ptr->update();

                  int uncollided_count = 0;
                  for(int i = 0; i < CL_Ignore_Colliders.count; i++)
                  {
                     auto entity = CL_Ignore_Colliders.list[i];
                     auto result = CL_test_player_vs_entity(entity, player);
                     if(result.collision) break;

                     uncollided_count++;
                  }

                  if(uncollided_count == CL_Ignore_Colliders.count)
                     break;

                  iteration++;
                  if(iteration == max_iterations)
                  {
                     // we couldn't unstuck the player in max_iterations * d_frame seconds of falling towards
                     // player movement direction, so he can't fall there
                     can_fall = false;
                     break;
                  }
               }

               if(can_fall)
               {
                  // final player position after falling from the edge (when he stops touching anything)
                  vec3 terminal_position = player->entity_ptr->position;
                  //IM_RENDER.add_mesh(IMHASH, &player->entity_ptr->collider);

                  auto& vel = player->entity_ptr->velocity;
                  if(abs(vel.x) < player->fall_from_edge_push_speed && abs(vel.z) < player->fall_from_edge_push_speed)
                     vel = player->v_dir_historic * player->fall_from_edge_push_speed;

                  GP_change_player_state(player, PLAYER_STATE_FALLING);
               }
               else
               {
                  RENDER_MESSAGE("Player won't fit if he falls here.", 1000);
               }

               player->entity_ptr->position = pos_0;

            }
         }

         player->update();

         CL_run_iterative_collision_detection(player);

         break;
      }
      case PLAYER_STATE_FALLING:
      {
         player->entity_ptr->velocity += G_FRAME_INFO.duration * player->gravity; 
         player->entity_ptr->position += player->entity_ptr->velocity * G_FRAME_INFO.duration;
         player->update();

         CL_run_iterative_collision_detection(player);

         break;
      }
      case PLAYER_STATE_JUMPING:
      {
         player->entity_ptr->velocity += G_FRAME_INFO.duration * player->gravity; 
         player->entity_ptr->position += player->entity_ptr->velocity * G_FRAME_INFO.duration;
         player->update();

         if (player->entity_ptr->velocity.y <= 0)
            GP_change_player_state(player, PLAYER_STATE_FALLING);

         CL_run_iterative_collision_detection(player);

         break;
      }
   }
   
}

//@todo - Rethink the name and purpose of this function
RaycastTest CL_do_c_vtrace(Player* player)
{
   auto downward_ray    = Ray{player->feet() + vec3{0.0f, PLAYER_STEPOVER_LIMIT, 0.0f}, -UNIT_Y};
   RaycastTest raytest  = test_ray_against_scene(downward_ray, RayCast_TestOnlyFromOutsideIn, player->entity_ptr);

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



vec3 GP_get_player_next_position_when_standing(Player* player)
{
   // updates player position
   auto& v           = player->entity_ptr->velocity;
   auto& v_dir       = player->v_dir;
   auto& state       = player->player_state;

   bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

   auto dt = G_FRAME_INFO.duration;

   if(v_dir.x != 0 || v_dir.y != 0 || v_dir.z != 0)
      player->v_dir_historic = v_dir;

   if(player->speed < 0.f || no_move_command)
      player->speed = 0;

   auto& speed    = player->speed;
   float d_speed  = player->acceleration * dt;

   bool stopped                         = (speed > 0 && no_move_command);

   float speed_limit;
   if(player->dashing)
      speed_limit = player->dash_speed;
   else if(player->walking)
      speed_limit = player->walk_speed;
   else
      speed_limit = player->run_speed;
   
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
   auto checkpoints = G_SCENE_INFO.active_scene->checkpoints;
   for(int i = 0; i < checkpoints.size(); i++)
   {
      auto checkpoint            = checkpoints[i];

      //@todo: do a cylinder vs cylinder or cylinder vs aabb test here
      ///auto triggered             = CL_test_cylinder_vs_cylinder(...);
      bool triggered = false;
      if(triggered)
      {
         RENDER_MESSAGE("CHECKPOINT", 1000);
         player->set_checkpoint(checkpoint);
      }
   }
}

// -------------------
// > LEDGE GRABBING
// -------------------

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
//             RENDER_MESSAGE("Vaulting failed.");
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