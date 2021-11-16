
vec3 GP_player_next_position(Player* player);
void GP_check_trigger_interaction               (Player* player);
bool GP_check_player_grabbed_ledge              (Player* player, Entity* entity);
bool GP_check_player_vaulting                   (Player* player);


vec3 GP_player_next_position(Player* player)
{
   // updates player position
   auto& v           = player->entity_ptr->velocity;
   auto& v_dir       = player->v_dir;
   auto& state       = player->player_state;

   bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

   auto dt = G_FRAME_INFO.duration;


   switch(state)
   {
      case PLAYER_STATE_STANDING:
      {
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

         v.y -=  player->fall_acceleration * dt;  // dampen player y speed (vf = v0 - g*t)
         break;
      }


      case PLAYER_STATE_FALLING:
      {
         v.y -= player->fall_acceleration * dt;  // dampen player y speed (vf = v0 - g*t)
         break;
      }

      
      case PLAYER_STATE_SLIDING:
      {
         if(player->jumping_from_slope)
         {
            player->player_state             = PLAYER_STATE_JUMPING;
            player->jumping_from_slope       = false;
            player->height_before_fall       = player->entity_ptr->position.y;
            player->entity_ptr->velocity     = player->v_dir * player->slide_jump_speed;
         }
         break;
      }
   }

   // update player position
   // player->prior_position           = player->entity_ptr->position;
   // player->entity_ptr->position     += v * dt;
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

bool CL_check_event_trigger_collision(Entity* trigger, Player* player)
{

   return false;
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

//             P_change_state(player, PLAYER_STATE_GRABBING, ps_args);
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

//             P_change_state(player, PLAYER_STATE_GRABBING, ps_args);
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

//          P_change_state(player, PLAYER_STATE_VAULTING, ps_args);
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