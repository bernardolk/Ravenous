
void GP_update_player_state                     (Player* &player);
vec3 GP_get_player_next_position_when_standing  (Player* player);
void GP_check_trigger_interaction               (Player* player);
bool GP_check_player_grabbed_ledge              (Player* player, Entity* entity);
bool GP_check_player_vaulting                   (Player* player);
RaycastTest CL_do_c_vtrace                      (Player* player);
bool GP_simulate_player_collision_in_falling_trajectory(Player* player, vec2 xz_velocity);
// bool GP_scan_for_terrain(vec3 center, float radius, vec2 orientation0, float angle, int subdivisions);
// bool GP_do_vtrace_for_terrain(vec3 vtrace_origin, float terrain_baseline_height, vec3 debug_color);


void GP_update_player_state(Player* &player)
{   
   switch(player->player_state)
   {
      case PLAYER_STATE_STANDING:
      {
         // compute player next position
         auto next_position = GP_get_player_next_position_when_standing(player);

         // MOVE PLAYER FORWARD
         player->entity_ptr->position = next_position;
         player->update();

         auto results = CL_test_and_resolve_collisions(player);

         bool collided_with_terrain = false;
         for (int i = 0; i < results.count; i ++)
         {
            auto result = results.results[i];
            collided_with_terrain = dot(result.normal, UNIT_Y) > 0;
            if(!collided_with_terrain)
               CL_wall_slide_player(player, result.normal);
            else
               player->last_terrain_contact_normal = result.normal;
         }
      

         // DO PLAYER VTRACE IN NEXT POSITION USING LAST REGISTERED CONTACT POINT
         vec3 player_btm_sphere_center = player->entity_ptr->position + vec3(0, player->radius, 0);
         vec3 contact_point =  player_btm_sphere_center + -player->last_terrain_contact_normal * player->radius;
         IM_RENDER.add_line(IMHASH, player_btm_sphere_center, contact_point, COLOR_YELLOW_1);

         auto c_vtrace = CL_do_c_vtrace(player);

         if(player->v_dir != vec3(0) && c_vtrace.hit && !collided_with_terrain)
         {
            if(c_vtrace.distance > 0.0002)
            {
               RENDER_MESSAGE("c_vtrace.distance: " + format_float_tostr(c_vtrace.distance, 10));
               player->entity_ptr->position.y -= c_vtrace.distance;
               player->update();
               results = CL_test_and_resolve_collisions(player);

               collided_with_terrain = false;
               for (int i = 0; i < results.count; i ++)
               {
                  auto result = results.results[i];
                  collided_with_terrain = dot(result.normal, UNIT_Y) > 0;
                  if(collided_with_terrain)
                     player->last_terrain_contact_normal = result.normal;
               }
            }
         }
         else if (!c_vtrace.hit)
         {
            /* If player was standing on something and we added the collider to the ignored colliders for collision,
               then, run a simulation, moving player like gravity would, testing for collision in each step and
               once we are not colliding with the terrain colliders anymore, check if player fits without colliding with
               anything in that position and then allow him to fall through.
               If he doesn't fit, then ignore the hole and let player walk through it.
            */

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
            }
            else
               RENDER_MESSAGE("Player won't fit if he falls here.", 1000);
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
            bool collided_with_terrain = dot(result.normal, UNIT_Y) > 0;
            if(!collided_with_terrain)
               CL_wall_slide_player(player, result.normal);
            else
               GP_change_player_state(player, PLAYER_STATE_STANDING);
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
            bool collided_with_terrain = dot(result.normal, UNIT_Y) > 0;
            if(!collided_with_terrain)
               CL_wall_slide_player(player, result.normal);
         }

         // @todo - need to include case that player touches inclined terrain
         //          in that case it should also stand (or fall from edge) and not
         //          directly fall.
         if(results.count > 0)
            GP_change_player_state(player, PLAYER_STATE_FALLING);

         else if (player->entity_ptr->velocity.y <= 0)
            GP_change_player_state(player, PLAYER_STATE_FALLING);

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