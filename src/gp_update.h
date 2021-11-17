
void GP_update_player_state                     (Player* &player);
vec3 GP_get_player_next_position_when_standing  (Player* player);
void GP_check_trigger_interaction               (Player* player);
bool GP_check_player_grabbed_ledge              (Player* player, Entity* entity);
bool GP_check_player_vaulting                   (Player* player);
RaycastTest CL_do_c_vtrace                      (Player* player);
bool GP_simulate_player_collision_in_falling_trajectory(Player* player, vec2 xz_velocity);
bool GP_scan_for_terrain(vec3 center, float radius, vec2 orientation0, float angle, int subdivisions);
bool GP_do_vtrace_for_terrain(vec3 vtrace_origin, float terrain_baseline_height, vec3 debug_color);


float PLAYER_STEPOVER_LIMIT   = 0.21;

void GP_update_player_state(Player* &player)
{   
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

         // compute player next position
         auto next_position = GP_get_player_next_position_when_standing(player);

         // // RAYCAST FORWARD TO DISABLE COLLISION IF FLOOR DETECTED
         // auto p_next_pos_fwd_periphery = next_position + player->v_dir_historic * player->radius;
         // auto f_vtrace_pos = vec3(p_next_pos_fwd_periphery.x, player->top().y + 1, p_next_pos_fwd_periphery.z);

         // auto v_trace_fwd_ray = Ray{f_vtrace_pos, -UNIT_Y};
         // RaycastTest fwd_vtrace = test_ray_against_scene(v_trace_fwd_ray);
         // bool fwd_hit_terrain = false;
         // if(fwd_vtrace.hit)
         // {
         //    auto hitpoint = point_from_detection(v_trace_fwd_ray, fwd_vtrace);
         //    // draw arrow
         //    IM_RENDER.add_line(IMHASH, hitpoint, hitpoint + UNIT_Y * 3.f, COLOR_RED_1);
         //    IM_RENDER.add_point(IMHASH, hitpoint, COLOR_RED_3);

         //    float delta_y = abs(player->feet().y - hitpoint.y);
         //    if(delta_y <= PLAYER_STEPOVER_LIMIT)
         //    {
         //       // disable collision with potential floor / terrain
         //       CL_Ignore_Colliders.add(fwd_vtrace.entity);
         //       fwd_hit_terrain = true;
         //    }
         // }

         /* Do a raycast (vertical trace) to find prospect of terrain according to the thresholds of steppable heights*/

         float v_trace_array_angle = 100;
         float v_trace_array_subdivisions = 30;
         vec2  v_trace_orientation = to2d_xz(rotate(player->v_dir_historic, -glm::radians(v_trace_array_angle / 2), UNIT_Y));

         auto f_vtrace_pos = vec3(next_position.x, player->feet().y, next_position.z);         
         bool fwd_hit_terrain = GP_scan_for_terrain(
            f_vtrace_pos, player->radius, v_trace_orientation, v_trace_array_angle, v_trace_array_subdivisions
         );

         // RAYCAST BACKWARD TO DISABLE COLLISION IF FLOOR DETECTED
         auto p_pos_bwd_periphery = player->entity_ptr->position - player->v_dir_historic * player->radius;
         auto b_vtrace_pos = vec3(p_pos_bwd_periphery.x, player->top().y + 1, p_pos_bwd_periphery.z);

         auto v_trace_bwd_ray = Ray{b_vtrace_pos, -UNIT_Y};
         RaycastTest bwd_vtrace = test_ray_against_scene(v_trace_bwd_ray);
         bool bwd_hit_terrain = false;
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
               bwd_hit_terrain = true;
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

            if(!(bwd_hit_terrain && fwd_hit_terrain) && CL_Ignore_Colliders.count > 0)
            {
               // give player a push if necessary
               float fall_momentum_intensity = player->speed;
               if(fall_momentum_intensity < player->fall_from_edge_push_speed)
                  fall_momentum_intensity = player->fall_from_edge_push_speed;


               vec2 fall_momentum_dir;
               if(fwd_hit_terrain) fall_momentum_dir = -to2d_xz(player->v_dir_historic);
               else                fall_momentum_dir =  to2d_xz(player->v_dir_historic);

               vec2 fall_momentum = fall_momentum_dir * fall_momentum_intensity;

               bool can_fall = GP_simulate_player_collision_in_falling_trajectory(player, fall_momentum);

               if(can_fall)
               {
                  player->entity_ptr->velocity = to3d_xz(fall_momentum);

                  GP_change_player_state(player, PLAYER_STATE_FALLING);
               }
               else
               {
                  RENDER_MESSAGE("Player won't fit if he falls here.", 1000);
               }
            }
         }

         player->update();

         CL_test_and_resolve_collisions(player);

         break;
      }
      case PLAYER_STATE_FALLING:
      {
         player->entity_ptr->velocity += G_FRAME_INFO.duration * player->gravity; 
         player->entity_ptr->position += player->entity_ptr->velocity * G_FRAME_INFO.duration;
         player->update();

         CL_test_and_resolve_collisions(player);

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

         if (player->entity_ptr->velocity.y <= 0)
            GP_change_player_state(player, PLAYER_STATE_FALLING);

         CL_test_and_resolve_collisions(player);

         break;
      }
   }
   
}


bool GP_scan_for_terrain(vec3 center, float radius, vec2 orientation0, float angle, int subdivisions)
{
   /* Does a circular array of raycasts according to parameters.
      The circle will be considered to be 'touching the ground', hence limits for stepping up or down are applied
      from the "center" arg y component.
  
      center: circle's center
      radius: circle radius
      orientation0: reference direction for first ray
      angle: angle span from 0 to 360
      subdivisions: controls how many rays to shoot
   */

      bool hit_terrain = false;
      vec3 orientation = normalize(to3d_xz(orientation0));
      float delta_angle = angle / subdivisions;
      float current_angle = 0;
      while(current_angle <= angle)
      {
         orientation = rotate(orientation, glm::radians(delta_angle), UNIT_Y);
         vec3 ray_origin = center + orientation * radius;
         // moves ray up a bit
         ray_origin.y = center.y + 1;

         vec3 color = COLOR_RED_1;
         if(current_angle <= angle / 2)
            color = COLOR_RED_2;

         bool hit = GP_do_vtrace_for_terrain(ray_origin, center.y, color);
         hit_terrain = hit_terrain || hit;

         current_angle += delta_angle;
      }

      return hit_terrain;
}


bool GP_do_vtrace_for_terrain(vec3 vtrace_origin, float terrain_baseline_height, vec3 debug_color = COLOR_RED_1)
{
   auto vtrace_ray = Ray{ vtrace_origin, -UNIT_Y };
   RaycastTest vtrace = test_ray_against_scene(vtrace_ray);
   if(vtrace.hit)
   {
      auto hitpoint = point_from_detection(vtrace_ray, vtrace);
      // draw arrow
      IM_RENDER.add_line (IMCUSTOMHASH(to_string(vtrace_origin)), hitpoint, vtrace_origin, debug_color);
      IM_RENDER.add_point(IMCUSTOMHASH(to_string(vtrace_origin)), hitpoint, debug_color);

      float delta_y = abs(terrain_baseline_height - hitpoint.y);
      if(delta_y <= PLAYER_STEPOVER_LIMIT)
      {
         // disable collision with potential floor / terrain
         CL_Ignore_Colliders.add(vtrace.entity);
         return true;
      }
   }

   return false;
}


bool GP_simulate_player_collision_in_falling_trajectory(Player* player, vec2 xz_velocity)
{
   /*    Simulates how it would be if player fell following the xz_velocity vector.
         If player can get in a position where he is not stuck, we allow him to fall. 
   */

   // configs
   float d_frame = 0.014;

   auto pos_0     = player->entity_ptr->position;
   vec3 vel       = to3d_xz(xz_velocity);

   float max_iterations = 120;

   IM_RENDER.add_point(IMHASH, player->entity_ptr->position, 2.0, false, COLOR_GREEN_1, 1);

   int iteration = 0;
   while(true)
   {
      vel += d_frame * player->gravity; 
      player->entity_ptr->position += vel * d_frame;
      IM_RENDER.add_point(IM_ITERHASH(iteration), player->entity_ptr->position, 2.0, true, COLOR_GREEN_1, 1);

      // @todo: This test is completely meaningless
      //        It is only valid for avoiding falling into holes in the ground but it doesn't properly test for
      //        more complicated siutations where player would fall and not have space to fit it
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
         // if enterede here, then we couldn't unstuck the player in max_iterations * d_frame seconds of falling towards
         // player movement direction, so he can't fall there
         player->entity_ptr->position = pos_0;
         player->entity_ptr->update();
         return false;
      }
   }

   player->entity_ptr->position = pos_0;
   player->entity_ptr->update();
   return true;
}

//@todo - Rethink the name and purpose of this function
RaycastTest CL_do_c_vtrace(Player* player)
{
   // stands for Central Vertical Trace, basically, look below player's center for something steppable (terrain)

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