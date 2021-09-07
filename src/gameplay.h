void handle_common_input                     (InputFlags flags, Player* &player);
void handle_movement_input                   (InputFlags flags, Player* &player, ProgramModeEnum pm);
void check_for_floor_transitions             (Player* player);
void check_trigger_interaction               (Player* player);
bool update_player_world_cells               (Player* player);
void make_player_slide                       (Player* player, Entity* ramp, bool slide_fall = false);
void make_player_jump                        (Player* player);
void make_player_jump_from_slope             (Player* player);
bool check_player_grabbed_ledge              (Player* player, Entity* entity);
void make_player_grab_ledge                  (Player* player, Entity* entity, vec2 normal_vec, vec3 final_position, float d);
bool check_player_vaulting                   (Player* player);
void make_player_vault_over_obstacle         (Player* player, Entity* entity, vec2 normal_vec, vec3 final_position);
bool check_for_sliding_slope_floor           (Player* player);


// -------
// @TODO: incorporate those in move_player call
// -------
void make_player_jump(Player* player)
{
   auto& v = player->entity_ptr->velocity;
   auto& v_dir = player->v_dir;
   bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

   if(no_move_command)
      player->jumping_upwards = true;
   // minimum jump range
   else if(square_LE(v, player->jump_horz_thrust))
      v = v_dir * player->jump_horz_thrust;
   
   player->player_state = PLAYER_STATE_JUMPING;
   player->anim_state = P_ANIM_JUMPING;
   player->height_before_fall = player->entity_ptr->position.y;
   v.y = player->jump_initial_speed;
}

void make_player_slide(Player* player, Entity* ramp, bool slide_fall)
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

void check_for_floor_transitions(Player* player)
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

bool check_for_sliding_slope_floor(Player* player)
{
   auto col_geometry = player->standing_entity_ptr->collision_geometry.slope;
   if(col_geometry.inclination > SLIDE_MAX_ANGLE)
   {
      make_player_slide(player, player->standing_entity_ptr, true);
      return true;
   }

   if(col_geometry.inclination > SLIDE_MIN_ANGLE)
   {
      make_player_slide(player, player->standing_entity_ptr);
      return true;
   }

   return false;
}


// -------------------
// > ACTION
// -------------------

void check_trigger_interaction(Player* player)
{
   auto checkpoints = G_SCENE_INFO.active_scene->checkpoints;
   for(int i = 0; i < checkpoints.size(); i++)
   {
      auto checkpoint            = checkpoints[i];
      auto triggered             = CL_check_event_trigger_collision(checkpoint, player->entity_ptr);
      if(triggered)
      {
         RENDER_MESSAGE("TRIGGERED", 1000);
         player->set_checkpoint(checkpoint);
      }
   }
}


// -------------------
// > LEDGE GRABBING
// -------------------

void check_player_grabbed_ledge(Player* player)
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

            make_player_grab_ledge(player, entity, test.normal_vec, future_pos, dr - test.overlap);
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

            make_player_grab_ledge(player, entity, test.normal_vec, future_pos, dr - test.overlap);
            return;
         }
      }
   }
}

void make_player_grab_ledge(Player* player, Entity* entity, vec2 normal_vec, vec3 final_position, float d)
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

void make_player_get_up_from_edge(Player* player)
{
   player->player_state          = PLAYER_STATE_VAULTING;
   player->anim_state            = P_ANIM_VAULTING;
   player->vaulting_entity_ptr   = player->grabbing_entity;
   player->grabbing_entity       = NULL;
}

// -------------------
// > VAULTING
// -------------------

bool check_player_vaulting(Player* player)
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
         
         make_player_vault_over_obstacle(player, entity, test.normal_vec, future_pos);
         return true;
      }
   }
   return false;
}

void make_player_vault_over_obstacle(Player* player, Entity* entity, vec2 normal_vec, vec3 final_position)
{
   vec3 rev_normal = rev_2Dnormal(normal_vec);

   player->player_state          = PLAYER_STATE_VAULTING;
   player->anim_state            = P_ANIM_VAULTING;
   player->anim_final_pos        = final_position;
   player->anim_orig_pos         = player->entity_ptr->position;
   player->entity_ptr->velocity  = vec3(0);
   player->anim_orig_dir         = nrmlz(to_xz(pCam->Front));
   player->anim_final_dir        = rev_normal;
   player->vaulting_entity_ptr   = entity;
}

void finish_vaulting(Player* player)
{
   G_INPUT_INFO.forget_last_mouse_coords  = true;
   G_INPUT_INFO.block_mouse_move          = false;
   player->player_state                   = PLAYER_STATE_STANDING;
   player->standing_entity_ptr            = player->vaulting_entity_ptr;
   player->vaulting_entity_ptr            = NULL;
   player->anim_finished_turning          = false;
}