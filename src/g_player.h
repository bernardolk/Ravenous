void GP_move_player(Player* player);

void GP_make_player_slide                       (Player* player, Entity* ramp, bool slide_fall = false);
void GP_make_player_jump_from_slope             (Player* player);
void GP_make_player_grab_ledge                  (Player* player, Entity* entity, vec2 normal_vec, vec3 final_position, float d);
void GP_make_player_vault_over_obstacle         (Player* player, Entity* entity, vec2 normal_vec, vec3 final_position);

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


void GP_make_player_slide(Player* player, Entity* ramp, bool slide_fall)
{
   std::cout << "SLIDE FALLING" << "\n";
   player->standing_entity_ptr         = ramp;
   auto height_check                   = CL_get_terrain_height_at_player(player->entity_ptr, ramp);
   player->entity_ptr->position.y      = height_check.overlap + player->half_height;
   // make player 'snap' to slope
   auto collision_geom                 = ramp->collision_geometry.slope;
   auto &pv                            = player->entity_ptr->velocity;
   // make camera (player) turn to face either up or down the slope

   //auto pv_2d = vec2(pv.x, pv.z);
   // if(G_SCENE_INFO.view_mode == FIRST_PERSON)
   // {
   //    auto t_2d = vec2(collision_geom.tangent.x, collision_geom.tangent.z);
   //    auto dot = glm::dot(pv_2d, t_2d);
   //    if(dot == 0) dot = 1;   // compensates for orthogonal v and tangent
   //    auto projected = (dot/glm::length2(t_2d))*t_2d;
   //    auto camera_dir = vec3(projected.x, G_SCENE_INFO.camera->Front.y, projected.y);
   //    camera_look_at(G_SCENE_INFO.camera, camera_dir, false);
   // }

   pv = player->slide_speed * collision_geom.tangent;
   if(slide_fall)
      player->player_state = PLAYER_STATE_SLIDE_FALLING;
   else
      player->player_state = PLAYER_STATE_SLIDING;
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
   if(col_geometry.inclination > SLIDE_MAX_ANGLE)
   {
      GP_make_player_slide(player, player->standing_entity_ptr, true);
      return true;
   }

   if(col_geometry.inclination > SLIDE_MIN_ANGLE)
   {
      GP_make_player_slide(player, player->standing_entity_ptr);
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

            GP_make_player_grab_ledge(player, entity, test.normal_vec, future_pos, dr - test.overlap);
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

            GP_make_player_grab_ledge(player, entity, test.normal_vec, future_pos, dr - test.overlap);
            return;
         }
      }
   }
}


void GP_make_player_grab_ledge(Player* player, Entity* entity, vec2 normal_vec, vec3 final_position, float d)
{
   vec3 rev_normal = rev_2Dnormal(normal_vec);

   // this will be an animation in the future
   float turn_angle = glm::degrees(vector_angle_signed(to2d_xz(pCam->Front), normal_vec)) - 180;
   camera_change_direction(pCam, turn_angle, 0.f);
   CL_snap_player(player, normal_vec, d);

   player->player_state          = PLAYER_STATE_GRABBING;
   player->grabbing_entity       = entity;
   player->entity_ptr->velocity  = vec3(0);
   // after we are able to move while grabbing the ledge, this should move away from here
   {
      player->anim_final_dir        = rev_normal;
      player->anim_final_pos        = final_position;
      player->anim_orig_pos         = player->entity_ptr->position;
      player->anim_orig_dir         = nrmlz(to_xz(pCam->Front));
      player->entity_ptr->velocity  = vec3(0);
   }
}


void GP_make_player_get_up_from_edge(Player* player)
{
   player->player_state          = PLAYER_STATE_VAULTING;
   player->anim_state            = P_ANIM_VAULTING;
   player->vaulting_entity_ptr   = player->grabbing_entity;
   player->grabbing_entity       = NULL;
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
         
         GP_make_player_vault_over_obstacle(player, entity, test.normal_vec, future_pos);
         return true;
      }
   }
   return false;
}


void GP_make_player_vault_over_obstacle(Player* player, Entity* entity, vec2 normal_vec, vec3 final_position)
{
   vec3 rev_normal = rev_2Dnormal(normal_vec);

   player->player_state             = PLAYER_STATE_VAULTING;
   player->anim_state               = P_ANIM_VAULTING;
   player->anim_final_pos           = final_position;
   player->anim_orig_pos            = player->entity_ptr->position;
   player->entity_ptr->velocity     = vec3(0);
   player->anim_orig_dir            = nrmlz(to_xz(pCam->Front));
   player->anim_final_dir           = rev_normal;
   player->vaulting_entity_ptr      = entity;
}


void GP_finish_vaulting(Player* player)
{
   G_INPUT_INFO.forget_last_mouse_coords  = true;
   G_INPUT_INFO.block_mouse_move          = false;
   player->player_state                   = PLAYER_STATE_STANDING;
   player->standing_entity_ptr            = player->vaulting_entity_ptr;
   player->vaulting_entity_ptr            = NULL;
   player->anim_finished_turning          = false;
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