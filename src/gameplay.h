void handle_common_input(InputFlags flags, Player* &player);
void handle_movement_input(InputFlags flags, Player* &player, ProgramModeEnum pm);
void update_player_state(Player* &player, WorldStruct* world);
void make_player_slide(Player* player, Entity* ramp, bool slide_fall = false);
void make_player_jump_from_slope(Player* player);
void run_collision_checks_standing(Player* player);
void run_collision_checks_falling(Player* player);
void reset_input_flags(InputFlags flags);
void mark_entity_checked(Entity* entity);
void resolve_collision(CollisionData collision, Player* player);
void check_for_floor_transitions(Player* player);
void check_trigger_interaction(Player* player);
bool update_player_world_cells(Player* player);
void recompute_collision_buffer_entities(Player* player);
void reset_collision_buffer_checks();
void move_player(Player* player);


bool update_player_world_cells(Player* player)
{
   // update player world cells
   auto offset1 = vec3{-1.0f * player->radius, -1.0f * player->half_height, -1.0f * player->radius};
   auto offset2 = vec3{player->radius, 0, player->radius};
   auto update_cells = World.update_entity_world_cells(player->entity_ptr, offset1, offset2);
   if(update_cells.status == OK)
   {
      return update_cells.entity_changed_cell;
   }
   else
   { 
      cout << update_cells.message << "\n";
      return false;
   }
}

void move_player(Player* player)
{
   auto& v = player->entity_ptr->velocity;
   auto& v_dir = player->v_dir;
   auto& state = player->player_state;

   bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

   if(player->speed < 0.f || no_move_command)
      player->speed = 0;

   // string v_dir_string = "speed: " + to_string(player->speed);
   // G_BUFFERS.rm_buffer->add(v_dir_string, 0);

   auto dt = G_FRAME_INFO.duration * G_FRAME_INFO.time_step;

   switch(state)
   {
      case PLAYER_STATE_STANDING:
      {
         auto& speed = player->speed;
         float d_speed = player->acceleration * dt;

         // deacceleration
         bool contrary_movement = !comp_sign(v_dir.x, v.x) || !comp_sign(v_dir.z, v.z);
         bool stopped_dashing = !player->dashing && square_GT(v + d_speed, player->run_speed);

         if((speed > 0 && no_move_command) || stopped_dashing)
            d_speed *= -1;            
         else if(player->dashing && square_GE(v + d_speed, player->dash_speed))
            d_speed = 0;

         speed += d_speed;

         // if no movement command is issued, v_dir = 0,0,0 !
         v = speed * v_dir;    

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

         // dampen player y speed (vf = v0 - g*t)
         v.y -=  player->fall_acceleration * dt;
         break;
      }
      case PLAYER_STATE_FALLING:
      {
         // dampen player y speed (vf = v0 - g*t)
         v.y -= player->fall_acceleration * dt;
         break;
      }
      case PLAYER_STATE_SLIDING:
      {
         if(player->jumping_from_slope)
         {
            player->jumping_from_slope = false;
            player->player_state = PLAYER_STATE_JUMPING;
            player->height_before_fall = player->entity_ptr->position.y;
            player->entity_ptr->velocity = player->v_dir * player->slide_jump_speed;
         }
         break;
      }
   }

   // update player position
   player->prior_position = player->entity_ptr->position;
   player->entity_ptr->position += v * dt;
   return;
}

// -----------------
// COLLISION BUFFER
// -----------------
void recompute_collision_buffer_entities(Player* player)
{
   // copies collision-check-relevant entity ptrs to a buffer
   // with metadata about the collision check for the entity
   auto collision_buffer = G_BUFFERS.entity_buffer->buffer;
   int entity_count = 0;
   for(int i = 0; i < player->entity_ptr->world_cells_count; i++)
   {
      auto cell = player->entity_ptr->world_cells[i];
      for(int j = 0; j < cell->count; j++)
      {
         auto entity = cell->entities[j];
         
         // adds to buffer only if not present already
         bool present = false;
         for(int k = 0; k < entity_count; k++)
            if(collision_buffer[k].entity->id == entity->id)
            {
               present = true;
               break;
            }

         if(!present)
         {
            collision_buffer[entity_count].entity = entity;
            collision_buffer[entity_count].collision_check = false;
            entity_count++;
            if(entity_count > COLLISION_BUFFER_CAPACITY) assert(false);
         }
      }
   }

   G_BUFFERS.entity_buffer->size = entity_count;
}

void reset_collision_buffer_checks()
{
   for(int i = 0; i < G_BUFFERS.entity_buffer->size; i++)
      G_BUFFERS.entity_buffer->buffer[i].collision_check = false;
}
 
void update_player_state(Player* &player, WorldStruct* world)
{
   Entity* &player_entity = player->entity_ptr;

   if(player->lives <= 0)
   {
      G_BUFFERS.rm_buffer->add("PLAYER DIED (height:" + format_float_tostr(player->fall_height_log, 2) + " m)", 3000);
      player->die();
      return;
   }

   switch(player->player_state)
   {
      case PLAYER_STATE_FALLING:
      {
         // test collision with every object in scene entities vector
         run_collision_checks_falling(player);
         break;
      }
      case PLAYER_STATE_STANDING:
      {
         // step 1: if player switched floors, either just change his ground or make him slide if applicable
         check_for_floor_transitions(player);

         // step 2: position player at terrain's height
         auto terrain = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         player->entity_ptr->position.y = terrain.overlap + player->half_height;

         // step 3: resolve possible collisions then do step 2 again
         // check for collisions with scene BUT with floor
         run_collision_checks_standing(player);

         terrain = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         player->entity_ptr->position.y = terrain.overlap + player->half_height;

         // step 4: check if player is still standing on terrain
         auto xz_check = get_horizontal_overlap_with_player(player->standing_entity_ptr, player);

         if(!xz_check.is_collided)
         {
            std::cout << "PLAYER FELL" << "\n";
            player_entity->velocity.y = - 1 * player->fall_speed;
            player_entity->velocity.x *= 0.5;
            player_entity->velocity.z *= 0.5;

            player->player_state = PLAYER_STATE_FALLING;
            player->height_before_fall = player_entity->position.y;
         }
         break;
      }
      case PLAYER_STATE_JUMPING:
      {
         /* remarks about the jump system:
            we set at input key_press time (input.h) a high velocity upward for the player
            at each frame we decrement a little bit from the y velocity component using delta frame time
            IDEALLY we would set our target jump height and let the math work itself out from there.
            For our prototype this should be fine.
         */
         if (player->entity_ptr->velocity.y <= 0)
         {
            player->jumping_upwards = false;
            player->entity_ptr->velocity.y = 0;
            player->player_state = PLAYER_STATE_FALLING;
         }

         // test collision with every object in scene entities vector
         run_collision_checks_falling(player);
         break;
      }
      case PLAYER_STATE_SLIDING:
      {
         assert(glm::length(player_entity->velocity) > 0);

         auto terrain = check_for_floor_below_player(player);
         
         if(terrain.hit && terrain.entity != player->standing_entity_ptr)
         {
            cout << "EVICTED FROM SLOPE (" << player->standing_entity_ptr->name << ") to (" << terrain.entity->name << ")\n";
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = terrain.entity;
            player->entity_ptr->velocity.y = 0;
            player->entity_ptr->position.y += terrain.distance;
            auto terrain_collision = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
            break;
         }

         // check for collisions with scene BUT with floor
         run_collision_checks_standing(player);

         // if player is still standing after collision resolutions, correct player height, else, make fall
         auto terrain_collision = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         if(terrain_collision.is_collided)
         {
            player->entity_ptr->position.y = terrain_collision.overlap + player->half_height;
         }
         else
         {
            // make player "slide" towards edge and fall away from floor
            std::cout << "PLAYER FELL (EVICTED)" << "\n";
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = NULL;
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
            player->height_before_fall = player_entity->position.y;
         }
         break;
      }
      case PLAYER_STATE_SLIDE_FALLING:
      {
         assert(glm::length(player_entity->velocity) > 0);

         auto terrain = check_for_floor_below_player(player);
         
         if(terrain.hit && terrain.entity != player->standing_entity_ptr)
         {
            cout << "EVICTED FROM SLOPE (" << player->standing_entity_ptr->name << ") to (" << terrain.entity->name << ")\n";
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = terrain.entity;
            player->entity_ptr->velocity.y = 0;
            player->entity_ptr->position.y += terrain.distance;
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
            break;
         }

         // ... or player fell out of slope
         auto terrain_collision = get_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         if(terrain_collision.is_collided)
         {
            player->entity_ptr->position.y = terrain_collision.overlap + player->half_height;
         }  
         else
         {
            // make player "slide" towards edge and fall away from floor
            std::cout << "PLAYER FELL (EVICTED)" << "\n";
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = NULL;
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
            player->height_before_fall = player_entity->position.y;
         }
         
         break;
      }
      // case PLAYER_STATE_FALLING_FROM_EDGE:
      // {
      //    // Here it is assumed player ALREADY has a velocity vec pushing him away from the platform he is standing on
      //    assert(glm::length(player_entity->velocity) > 0);
      //    // check if still colliding with floor, if so, let player keep sliding, if not, change to FALLING
      //    Collision c_test;
      //    c_test = get_horizontal_overlap_with_player(player->standing_entity_ptr, player);
            
      //    if(!c_test.is_collided)
      //    {
      //       player->player_state = PLAYER_STATE_FALLING;
      //       player->standing_entity_ptr = NULL;
      //       // player_entity->velocity = vec3(0, 0, 0); 
      //    }
      //    break;
      // }
      case PLAYER_STATE_EVICTED_FROM_SLOPE:
      {
         // here, player can already be considered standing somewhere or not. Which is weird.
         assert(player->slope_player_was_ptr != NULL);
         // Here it is assumed player ALREADY has a velocity vec pushing him away from the slope
         assert(glm::length(player_entity->velocity) > 0);
         // check if still colliding with floor, if so, let player keep sliding, if not, change to FALLING
         auto c_test = get_horizontal_overlap_with_player(player->slope_player_was_ptr, player);
         if(!c_test.is_collided)
         {
            player->slope_player_was_ptr = NULL;
            if(player->standing_entity_ptr != NULL)
               player->player_state = PLAYER_STATE_STANDING;
            else
            {
               player->entity_ptr->velocity.x = 0;
               player->entity_ptr->velocity.z = 0;
               player->player_state = PLAYER_STATE_FALLING;
            }
         }
         break;
      }
   }
} 


void check_for_floor_transitions(Player* player)
{
   // this proc is used when player is standing

   auto terrain = check_for_floor_below_player(player);
   
   if(terrain.hit && terrain.entity != player->standing_entity_ptr)
   {
      if(terrain.entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
      {
         // player can only keep standing if ramp is standable
         if(terrain.entity->collision_geometry.slope.inclination < SLIDE_MIN_ANGLE)
            player->standing_entity_ptr = terrain.entity;
      }
      else
         player->standing_entity_ptr = terrain.entity;
   }
}

// ________________________________________________________________________________
//
// SCENE COLLISION CONTROLLER FUNCTIONS - ITERATIVE MULTI-CHECK COLLISION DETECTION 
// ________________________________________________________________________________

void run_collision_checks_standing(Player* player)
{
   auto entity_buffer = G_BUFFERS.entity_buffer;
   while(true)
   {
      auto buffer = entity_buffer->buffer;  // places pointer back to start
      CollisionData collision_data = check_collision_horizontal(player, buffer, entity_buffer->size);
      if(collision_data.collided_entity_ptr != NULL)
      {
         mark_entity_checked(collision_data.collided_entity_ptr);
         resolve_collision(collision_data, player);
      }
      else break;
   }
}

void run_collision_checks_falling(Player* player)
{
   // Here, we will check first for vertical intersections to see if player needs to be teleported 
   // to the top of the entity it has collided with. 
   // If so, we deal with it on the spot (teleports player) and then keep checking for other collisions
   // to be resolved once thats done. Horizontal checks come after vertical collisions because players 
   // shouldnt loose the chance to make their jump because we are preventing them from getting stuck first.

   auto entity_buffer = G_BUFFERS.entity_buffer;  
   while(true)
   {
      bool any_collision = false;
      // CHECKS VERTICAL COLLISIONS
      {
         auto buffer = entity_buffer->buffer;  // places pointer back to start
         auto v_collision_data = check_collision_vertical(player, buffer, entity_buffer->size);
         if(v_collision_data.collided_entity_ptr != NULL)
         {
            any_collision = true;
            mark_entity_checked(v_collision_data.collided_entity_ptr);
            resolve_collision(v_collision_data, player);
         }

         buffer = entity_buffer->buffer;  // places pointer back to start
         auto h_collision_data = check_collision_horizontal(player, buffer, entity_buffer->size);
         if(h_collision_data.collided_entity_ptr != NULL)
         {
            any_collision = true;
            mark_entity_checked(h_collision_data.collided_entity_ptr);
            resolve_collision(h_collision_data, player);
         }

         if(!any_collision) break;
      }
   }
}

void mark_entity_checked(Entity* entity)
{
   // marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
   auto entity_buffer = G_BUFFERS.entity_buffer;
   auto entity_element = entity_buffer->buffer;
   for(int i = 0; i < entity_buffer->size; ++i)
   {
      if(entity_element->entity == entity)
      {
         entity_element->collision_check = true;
         break;
      }
      entity_element++;
   }
}

void resolve_collision(CollisionData collision, Player* player)
{
   // the point of this is to not trigger health check 
   // when player hits a wall while falling
   bool trigger_check_was_player_hurt = false;

   // if collided, unset mid-air controls
   player->jumping_upwards = false;
   switch(collision.collision_outcome)
   {
      case JUMP_SUCCESS:
      {
         trigger_check_was_player_hurt = true;
         std::cout << "LANDED" << "\n";
         // move player to surface, stop player and set him to standing
         player->standing_entity_ptr = collision.collided_entity_ptr;
         auto height_check = get_terrain_height_at_player(player->entity_ptr, player->standing_entity_ptr);
         player->entity_ptr->position.y = height_check.overlap + player->half_height; 
         player->entity_ptr->velocity = vec3(0,0,0);
         player->player_state = PLAYER_STATE_STANDING;
         break;
      }
      case JUMP_FACE_FLAT:
      {
         std::cout << "JUMP FACE FLAT" << "\n";
         // deals with collision
         // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
         player->entity_ptr->position -= 
               vec3(collision.normal_vec.x, 0, collision.normal_vec.y)  * collision.overlap;

         // make player slide through the tangent of platform
         auto tangent_vec = vec2(collision.normal_vec.y, collision.normal_vec.x);      
         auto v_2d = vec2(player->entity_ptr->velocity.x, player->entity_ptr->velocity.z);
         auto project = (glm::dot(v_2d, tangent_vec)/glm::length2(tangent_vec))*tangent_vec;

         player->entity_ptr->velocity.x = project.x;
         player->entity_ptr->velocity.z = project.y; 
               
         if(player->player_state == PLAYER_STATE_JUMPING)
         {
            player->player_state = PLAYER_STATE_FALLING;
            player->entity_ptr->velocity.y = 0;
         }

         break;
      }
      case JUMP_SLIDE:
      {
         trigger_check_was_player_hurt = true;
         make_player_slide(player, collision.collided_entity_ptr);
         break;
      }
      case JUMP_SLIDE_HIGH_INCLINATION:
      {
         trigger_check_was_player_hurt = true;
         make_player_slide(player, collision.collided_entity_ptr, true);
         break;
      }
      case JUMP_CEILING:
      {
         std::cout << "HIT CEILING" << "\n";
         player->entity_ptr->position.y -= collision.overlap + COLLISION_EPSILON; 
         player->player_state = PLAYER_STATE_FALLING;
         player->entity_ptr->velocity.y = 0;
         break; 
      }
      case STEPPED_SLOPE:
      {
         // cout << "PLAYER STEPPED INTO SLOPE \n";
         player->standing_entity_ptr = collision.collided_entity_ptr;
         player->entity_ptr->position.y += collision.overlap;
         break;
      }
      case BLOCKED_BY_WALL:
      {
         // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
         player->entity_ptr->position -= vec3(
            collision.normal_vec.x, 0, collision.normal_vec.y
         ) * collision.overlap;
         break;
      }
   }

   // hurts player if necessary
   if(trigger_check_was_player_hurt) player->maybe_hurt_from_fall();
}


void make_player_slide(Player* player, Entity* ramp, bool slide_fall)
{
   std::cout << "SLIDE FALLING" << "\n";
   player->standing_entity_ptr = ramp;
   auto height_check = get_terrain_height_at_player(player->entity_ptr, ramp);
   player->entity_ptr->position.y = height_check.overlap + player->half_height;
   // make player 'snap' to slope
   auto collision_geom = ramp->collision_geometry.slope;
   auto &pv = player->entity_ptr->velocity;
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


void check_trigger_interaction(Player* player)
{
   auto checkpoints = G_SCENE_INFO.active_scene->checkpoints;
   for(int i = 0; i < checkpoints.size(); i++)
   {
      auto checkpoint = checkpoints[i];
      auto triggered = check_event_trigger_collision(checkpoint, player->entity_ptr);
      if(triggered)
      {
         G_BUFFERS.rm_buffer->add("TRIGGERED", 1000);
         player->set_checkpoint(checkpoint);
      }
   }
}


void handle_common_input(InputFlags flags, Player* &player)
{
   if(pressed_once(flags, KEY_COMMA))
   {
      if(G_FRAME_INFO.time_step > 0)
      {
         G_FRAME_INFO.time_step -= 0.025; 
      }
   }
   if(pressed_once(flags, KEY_PERIOD))
   {
      if(G_FRAME_INFO.time_step < 3)
      {
         G_FRAME_INFO.time_step += 0.025;
      }
   }
   if(pressed_once(flags, KEY_1))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x0.05", 1000);
      G_FRAME_INFO.time_step = 0.05;
   }
   if(pressed_once(flags, KEY_2))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x0.1", 1000);
      G_FRAME_INFO.time_step = 0.1;
   }
   if(pressed_once(flags, KEY_3))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x1.0", 1000);
      G_FRAME_INFO.time_step = 1.0;
   }
   if(pressed_once(flags, KEY_4))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x0.3", 1000);
      G_FRAME_INFO.time_step = 0.3;
   }
   if(pressed_once(flags, KEY_5))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x2.0", 1000);
      G_FRAME_INFO.time_step = 2.0;
   }
   if(flags.key_press & KEY_K)
   {
      player->die();
   }
   if(pressed_once(flags, KEY_F))
   {
      toggle_program_modes(player);
   }
   if(pressed_once(flags, KEY_J))
   {
      check_trigger_interaction(player);
   }
   if(flags.key_press & KEY_ESC && flags.key_press & KEY_LEFT_SHIFT)
   {
       glfwSetWindowShouldClose(G_DISPLAY_INFO.window, true);
   }
}

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
   player->height_before_fall = player->entity_ptr->position.y;
   v.y = player->jump_initial_speed;
}

void handle_movement_input(InputFlags flags, Player* &player, ProgramModeEnum pm)
{
   // assign keys
   u64 MOV_UP, MOV_DOWN, MOV_LEFT, MOV_RIGHT;
   switch(pm)
   {
      case EDITOR_MODE:
         MOV_UP    = KEY_UP;
         MOV_DOWN  = KEY_DOWN;
         MOV_LEFT  = KEY_LEFT;
         MOV_RIGHT = KEY_RIGHT;
         break;
      case GAME_MODE:
         MOV_UP    = KEY_W;
         MOV_DOWN  = KEY_S;
         MOV_LEFT  = KEY_A;
         MOV_RIGHT = KEY_D;
         break;
   }

   // reset player movement intention state
   player->dashing = false;
   auto& v = player->v_dir;
   v = vec3(0);

   // combines all key presses into one v direction
   switch(player->player_state)
   {
      case PLAYER_STATE_JUMPING:
      {
         // MID-AIR CONTROL IF JUMPING UP
         if(player->jumping_upwards)
         {
            if(pressed(flags, MOV_UP))
            {
               v += vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
            }
            if(pressed(flags, MOV_LEFT))
            {
               vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
               v -= vec3(onwards_vector.x, 0, onwards_vector.z);
            }
            if(pressed(flags, MOV_DOWN))
            {
               v -= vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
            }
            if(pressed(flags, MOV_RIGHT))
            {
               vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
               v += vec3(onwards_vector.x, 0, onwards_vector.z);
            }
         }
         break;
      }
      case PLAYER_STATE_STANDING:
      {
         // MOVE
         if(pressed(flags, MOV_UP))
         {
            v += vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
         }
         if(pressed(flags, MOV_LEFT))
         {
            vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
            v -= vec3(onwards_vector.x, 0, onwards_vector.z);
         }
         if(pressed(flags, MOV_DOWN))
         {
            v -= vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
         }
         if(pressed(flags, MOV_RIGHT))
         {
            vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
            v += vec3(onwards_vector.x, 0, onwards_vector.z);
         }
         
         // DASH
         if(flags.key_press & KEY_LEFT_SHIFT)  
            player->dashing = true;
         
         // JUMP
         if (flags.key_press & KEY_SPACE) 
            make_player_jump(player);

         break;
      }
      case PLAYER_STATE_SLIDING:
      {
         auto collision_geom = player->standing_entity_ptr->collision_geometry.slope;
         v = player->slide_speed * collision_geom.tangent;

         if (flags.key_press & MOV_LEFT)
         {
            float dot_product = glm::dot(collision_geom.tangent, G_SCENE_INFO.camera->Front);
            float angle = -12.0f;
            if (dot_product < 0)
               angle *= -1;

            auto bitangent = glm::cross(collision_geom.tangent, G_SCENE_INFO.camera->Up);
            auto normal = glm::cross(bitangent, collision_geom.tangent);
            auto temp_vec = glm::rotate(v, angle, normal);
            v.x = temp_vec.x;
            v.z = temp_vec.z;
         }
         if (flags.key_press & MOV_RIGHT)
         {
            float dot_product = glm::dot(collision_geom.tangent, G_SCENE_INFO.camera->Front);
            float angle = 12.0f;
            if (dot_product < 0)
               angle *= -1;

            auto bitangent = glm::cross(collision_geom.tangent, G_SCENE_INFO.camera->Up);
            auto normal = glm::cross(bitangent, collision_geom.tangent);
            auto temp_vec = glm::rotate(v, angle, normal);
            v.x = temp_vec.x;
            v.z = temp_vec.z;
         }
         if (flags.key_press & KEY_SPACE)
         {
            vec3 n = player->standing_entity_ptr->collision_geometry.slope.normal;
            player->jumping_from_slope = true;
            v = glm::normalize(vec3(n.x, 1, n.z));
         }

         break;
      }
   }

   if(!(v.x == 0.f && v.y == 0.f && v.z == 0.f))
      v = glm::normalize(v);

   move_player(player);
}

void reset_input_flags(InputFlags flags)
{
   // here we record a history for if keys were last pressed or released, so to enable smooth toggle
   G_INPUT_INFO.key_state |= flags.key_press;
   G_INPUT_INFO.key_state &= ~(flags.key_release); 
}