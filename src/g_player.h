void GP_move_player(Player* player);

void GP_check_trigger_interaction               (Player* player);
void GP_check_for_floor_transitions             (Player* player);
bool GP_check_player_grabbed_ledge              (Player* player, Entity* entity);
bool GP_check_player_vaulting                   (Player* player);
bool GP_check_for_sliding_slope_floor           (Player* player);



void GP_move_player(Player* player)
{
   // updates player position
   auto& v           = player->entity_ptr->velocity;
   auto& v_dir       = player->v_dir;
   auto& state       = player->player_state;

   bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

   if(player->speed < 0.f || no_move_command)
      player->speed = 0;

   auto dt = G_FRAME_INFO.duration;


   switch(state)
   {
      case PLAYER_STATE_STANDING:
      {
         auto& speed    = player->speed;
         float d_speed  = player->acceleration * dt;

         // deacceleration
         bool stopped                         = (speed > 0 && no_move_command);
         bool moving_contrary_to_momentum     = !comp_sign(v_dir.x, v.x) || !comp_sign(v_dir.z, v.z);
         bool stopped_dashing                 = !player->dashing && square_GT(v + d_speed, player->run_speed);
         bool started_walking_while_fast      = player->walking  && square_GT(v + d_speed, player->walk_speed);
         bool cap_speed_dashing               = player->dashing  && square_GE(v + d_speed, player->dash_speed);
         bool cap_speed_walking               = player->walking  && square_GE(v + d_speed, player->walk_speed);
         bool cap_speed_running               = !(player->dashing || player->walking)  && square_GE(v + d_speed, player->run_speed);

         // if(stopped || stopped_dashing || started_walking_while_fast || moving_contrary_to_momentum)
         // { d_speed   *= -1; }            
        
         if(stopped)
         { speed = 0; d_speed = 0; }
         else if(cap_speed_dashing || cap_speed_walking || cap_speed_running)  
         { d_speed   = 0; }

         speed += d_speed;
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
   player->prior_position           = player->entity_ptr->position;
   player->entity_ptr->position     += v * dt;
   return;
}


// ----------------------
// > FLOOR RELATED STUFF
// ----------------------

void GP_check_for_floor_transitions(Player* player)
{
   // this proc is used when player is standing

   auto terrain = CL_check_for_floor_below_player(player);
   
   if(terrain.hit && terrain.entity != player->standing_entity_ptr)
   {
      // if(terrain.entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
      // {
      //    // player can only keep standing if ramp is standable
      //    if(terrain.entity->collision_geometry.slope.inclination < SLIDE_MIN_ANGLE)
      //       player->standing_entity_ptr = terrain.entity;
      // }
      // else
         player->standing_entity_ptr = terrain.entity;
   }
}

void GP_check_for_floor_transitions_while_walking(Player* player)
{
   // this proc is used when player is standing and walking
   // avoids player stepping into abyss or slopes when walking

   auto terrain = CL_check_for_floor_below_player(player);
   
   if(terrain.hit && terrain.entity != player->standing_entity_ptr 
      && terrain.entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
   {
      player->standing_entity_ptr = terrain.entity;
   }
   else if(!(terrain.hit && terrain.entity == player->standing_entity_ptr))
   {
      player->entity_ptr->position = player->prior_position;
   }
}


bool GP_check_for_sliding_slope_floor(Player* player)
{
   auto col_geometry = player->standing_entity_ptr->collision_geometry.slope;

   if(col_geometry.inclination > SLIDE_MIN_ANGLE)
   {
      PlayerStateChangeArgs ps_args;
      ps_args.entity = player->standing_entity_ptr;

      if(col_geometry.inclination > SLIDE_MAX_ANGLE)
         P_change_state(player, PLAYER_STATE_SLIDE_FALLING, ps_args);
      else
         P_change_state(player, PLAYER_STATE_SLIDING);

      return true;
   }

   return false;
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
      auto triggered             = CL_check_event_trigger_collision(checkpoint, player->entity_ptr);
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

void GP_check_player_grabbed_ledge(Player* player)
{
   // ledge grab y tollerance
   const float y_tol = 0.1;
   // half the ledge grab semicircle region angle, in degrees 
   const float s_theta = 40;
   // radius of detection
   const float dr = 0.1;

   float player_y = player->top().y;
   auto camera_f = vec2(pCam->Front.x, pCam->Front.z);


   for(int i = 0; i < G_BUFFERS.entity_buffer->size; i++)
   {
      Entity* entity = G_BUFFERS.entity_buffer->buffer[i].entity;

      if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
      {
         float edge_y = entity->position.y + entity->get_height();
         if(!(player_y < edge_y + y_tol && player_y > edge_y - y_tol))
            continue;

         auto [x0, x1, z0, z1] = entity->get_rect_bounds();
         auto test = CL_circle_vs_square(
            player->entity_ptr->position.x, 
            player->entity_ptr->position.z, 
            player->radius + dr,
            x0, x1, z0, z1
         );

         if(!test.is_collided)
            continue;

         float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
         float min_theta = 180 - s_theta;
         float max_theta = 180 + s_theta;
         if(min_theta <= theta && theta <= max_theta)
         {
            // checks if area above ledge is free for standing
            vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
            IM_RENDER.add_mesh(IMHASH, player->entity_ptr, future_pos);
            if(CL_test_in_mock_position(player, future_pos))
               continue;

            PlayerStateChangeArgs ps_args;
            ps_args.entity = entity;
            // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
            ps_args.final_position = future_pos;
            ps_args.penetration = dr - test.overlap;

            P_change_state(player, PLAYER_STATE_GRABBING, ps_args);
            return;
         }
      }

      else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
      {
         float edge_y = entity->position.y;
         if(!(player_y < edge_y + y_tol && player_y > edge_y - y_tol))
            continue;

         auto [x0, x1, z0, z1] = entity->get_rect_bounds();
         auto test = CL_circle_vs_square(
            player->entity_ptr->position.x, 
            player->entity_ptr->position.z, 
            player->radius + dr,
            x0, x1, z0, z1
         );

         if(!test.is_collided)
            continue;

         // player is not facing slope's inclined face
         if(get_slope_normal(entity) != test.normal_vec)
            continue;

         float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
         float min_theta = 180 - s_theta;
         float max_theta = 180 + s_theta;
         if(min_theta <= theta && theta <= max_theta)
         {
            // checks if area above ledge is free for standing
            vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
            IM_RENDER.add_mesh(IMHASH, player->entity_ptr, future_pos);
            if(CL_test_in_mock_position(player, future_pos, entity))
               continue;

            PlayerStateChangeArgs ps_args;
            ps_args.entity = entity;
            // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
            ps_args.final_position = future_pos;
            ps_args.penetration = dr - test.overlap;

            P_change_state(player, PLAYER_STATE_GRABBING, ps_args);
            return;
         }
      }
   }
}


// -------------------
// > VAULTING
// -------------------

bool GP_check_player_vaulting(Player* player)
{
   // action cone half theta 
   const float s_theta = 40;
   // radius of detection
   const float dr = 0.1;

   float player_y = player->entity_ptr->position.y;
   auto camera_f = vec2(pCam->Front.x, pCam->Front.z);

   for(int i = 0; i < G_BUFFERS.entity_buffer->size; i++)
   {
      Entity* entity = G_BUFFERS.entity_buffer->buffer[i].entity;

      if(entity->collision_geometry_type != COLLISION_ALIGNED_BOX)
         continue;

      float rel_height = (entity->position.y + entity->get_height()) - player->feet().y;

      // short platforms should be ignored since we will use navigation meshes that include them smoothly with a nav ramp 
      // and therefore going over them do not count as 'vaulting moves'
      if(rel_height < 0.3) // also makes sure we only get positive rel heights
         continue;
      
      if(rel_height >= player->half_height * 2)
         continue;

      auto [x0, x1, z0, z1] = entity->get_rect_bounds();
      auto test = CL_circle_vs_square(
         player->entity_ptr->position.x,
         player->entity_ptr->position.z,
         player->radius + dr,
         x0, x1, z0, z1
      );

      if(!test.is_collided)
         continue;

      float theta = glm::degrees(vector_angle(camera_f, test.normal_vec));
      float min_theta = 180 - s_theta;
      float max_theta = 180 + s_theta;
      if(min_theta <= theta && theta <= max_theta)
      {
         // checks if area above ledge is free for standing
         vec3 future_pos = CL_player_future_pos_obstacle(player, entity, test.normal_vec, dr - test.overlap);
         // IM_RENDER.add_mesh(IMHASH, player->entity_ptr, future_pos);
         if(CL_test_in_mock_position(player, future_pos))
         {
            RENDER_MESSAGE("Vaulting failed.");
            continue;
         }
         
         PlayerStateChangeArgs ps_args;
         ps_args.entity = entity;
         // ps_args.normal = test.normal_vec;   // NOPE - should use proper test normal
         ps_args.final_position = future_pos;
         ps_args.penetration = dr - test.overlap;

         P_change_state(player, PLAYER_STATE_VAULTING, ps_args);
         return true;
      }
   }
   return false;
}


void GP_check_player_events(Player* player)
{
   // Player death
   if(player->lives <= 0)
   {
      G_BUFFERS.rm_buffer->add("PLAYER DIED (height:" + format_float_tostr(player->fall_height_log, 2) + " m)", 3000);
      player->die();
      return;
   }
}