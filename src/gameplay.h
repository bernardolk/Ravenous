void handle_input_flags(InputFlags flags, Player* &player);
void update_player_state(Player* &player);
void make_player_slide(Player* player, CollisionData collision_data);
void make_player_slide_fall(Player* player, CollisionData collision_data);
void run_collision_checks_standing(Player* player, size_t entity_list_size);
void run_collision_checks_falling(Player* player, size_t entity_list_size);
CollisionData check_collision_horizontal(
      Player* player, EntityBufferElement* entity_iterator, size_t entity_list_size
); 
CollisionData check_collision_vertical(Player* player, EntityBufferElement* entity_iterator, size_t entity_list_size);
void player_death_handler(Player* &player);
void game_handle_input_flags(InputFlags flags, Player* &player);
void reset_input_flags(InputFlags flags);


float SLIDE_MAX_ANGLE = 1.4;
float SLIDE_MIN_ANGLE = 0.6;

void update_player_state(Player* &player)
{
   Entity* &player_entity = player->entity_ptr;

   if(player->lives <= 0)
   {
      player_death_handler(player);
      return;
   }

   // makes player move and register player last position
   player->prior_position = player_entity->position;
   player_entity->position += player_entity->velocity * G_FRAME_INFO.delta_time * G_FRAME_INFO.time_step;

   switch(player->player_state)
   {
      case PLAYER_STATE_FALLING:
      {
         player->entity_ptr->velocity.y -= G_FRAME_INFO.delta_time * player->fall_acceleration * G_FRAME_INFO.time_step;

         // test collision with every object in scene entities vector
         size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();
         run_collision_checks_falling(player, entity_list_size);
         break;
      }
      case PLAYER_STATE_STANDING:
      {

         // step 1: position player height
         auto terrain_collision = sample_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         player->entity_ptr->position.y = terrain_collision.overlap + player->half_height;

         // step 2: resolve possible collisions
         size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();
         // check for collisions with scene BUT with floor
         run_collision_checks_standing(player, entity_list_size);

         // step 3: check if player is still standing

         // we sample again, after solving collisions
         auto players_terrain = sample_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         player->entity_ptr->position.y = players_terrain.overlap + player->half_height;
         
         if(!players_terrain.is_collided)
         {
            CollisionData check;
            switch(player->standing_entity_ptr->collision_geometry_type)
            {
               case COLLISION_ALIGNED_BOX:
                  check = check_for_floor_below_player(player);
                  break;
               case COLLISION_ALIGNED_SLOPE:
                  check = check_for_floor_below_player_when_slope(player);
                  break;
               default:
                  assert(false);
            }

            bool player_fell = false;

            if(!check.is_collided) player_fell = true;
            
            else
            {
               if(check.collided_entity_ptr->collision_geometry_type == COLLISION_ALIGNED_BOX)
               {
                  player->standing_entity_ptr = check.collided_entity_ptr;
               }
               else if(check.collided_entity_ptr->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
               {
                  auto collision_geometry = check.collided_entity_ptr->collision_geometry.slope;
                  if(collision_geometry.inclination < SLIDE_MIN_ANGLE)
                  {
                     player->standing_entity_ptr = check.collided_entity_ptr;
                  }
                  else player_fell = true;
               }
            }

            if(player_fell)
            {
               // make player "slide" towards edge and fall away from floor
               std::cout << "PLAYER FELL" << "\n";
               player_entity->velocity.y = - 1 * player->fall_speed;
               player->player_state = PLAYER_STATE_FALLING_FROM_EDGE;
               player->height_before_fall = player_entity->position.y;
            }
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
         //dampen player speed (vf = v0 - g*t)
         player->entity_ptr->velocity.y -= G_FRAME_INFO.delta_time * player->fall_acceleration * G_FRAME_INFO.time_step;
         if (player->entity_ptr->velocity.y <= 0)
         {
            player->entity_ptr->velocity.y = 0;
            player->player_state = PLAYER_STATE_FALLING;
         }

         // test collision with every object in scene entities vector
         size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();
         run_collision_checks_falling(player, entity_list_size);
         break;
      }
      case PLAYER_STATE_SLIDING:
      {
         assert(glm::length(player_entity->velocity) > 0);

         // check if collided with another floor
         auto floor_check = check_for_floor_below_player_when_slope(player, true);

         if(floor_check.is_collided)
         {
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = floor_check.collided_entity_ptr;
            player->entity_ptr->velocity.y = 0;
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
            break;
         }

         size_t entity_list_size = G_SCENE_INFO.active_scene->entities.size();
         
         // check for collisions with scene BUT with floor
         run_collision_checks_standing(player, entity_list_size);

         // correct player position in case collided with a wall
         auto terrain_collision = sample_terrain_height_at_player(player_entity, player->standing_entity_ptr);
         if(terrain_collision.is_collided)
         {
            player->entity_ptr->position.y = terrain_collision.overlap + player->half_height;
         }
         else
         {
            // make player "slide" towards edge and fall away from floor
            std::cout << "PLAYER FELL" << "\n";
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

         auto floor_check = check_for_floor_below_player_when_slope(player, true);

         if(floor_check.is_collided)
         {
            player->slope_player_was_ptr = player->standing_entity_ptr;
            player->standing_entity_ptr = floor_check.collided_entity_ptr;
            player->entity_ptr->velocity.y = 0;
            player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
         }

         // player fell out of slope
         else
         {
            auto terrain_collision = sample_terrain_height_at_player(player_entity, player->standing_entity_ptr);
            if(terrain_collision.is_collided)
            {
               player->entity_ptr->position.y = terrain_collision.overlap + player->half_height;
            }  
            else
            {
               // make player "slide" towards edge and fall away from floor
               std::cout << "PLAYER FELL" << "\n";
               player->slope_player_was_ptr = player->standing_entity_ptr;
               player->standing_entity_ptr = NULL;
               player->player_state = PLAYER_STATE_EVICTED_FROM_SLOPE;
               player->height_before_fall = player_entity->position.y;
            }
         }
      
         break;
      }
      case PLAYER_STATE_FALLING_FROM_EDGE:
      {
         // Here it is assumed player ALREADY has a velocity vec pushing him away from the platform he is standing on
         assert(glm::length(player_entity->velocity) > 0);
         // check if still colliding with floor, if so, let player keep sliding, if not, change to FALLING
         Collision c_test;
         c_test = get_horizontal_overlap_with_player(player->standing_entity_ptr, player);
            
         if(!c_test.is_collided)
         {
            player->player_state = PLAYER_STATE_FALLING;
            player->standing_entity_ptr = NULL;
            // player_entity->velocity = vec3(0, 0, 0); 
         }
         break;
      }
      case PLAYER_STATE_EVICTED_FROM_SLOPE:
      {
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

// ________________________________________________________________________________
//
// SCENE COLLISION CONTROLLER FUNCTIONS - ITERATIVE MULTI-CHECK COLLISION DETECTION 
// ________________________________________________________________________________

void run_collision_checks_standing(Player* player, size_t entity_list_size)
{
   auto entity_buffer = (EntityBuffer*)G_BUFFERS.buffers[0];
   bool end_collision_checks = false;
   while(!end_collision_checks)
   {
      auto entity_buf_iter = entity_buffer->buffer;  // places pointer back to start
      CollisionData collision_data = check_collision_horizontal(player, entity_buf_iter, entity_list_size);
      if(collision_data.collided_entity_ptr != NULL)
      {
         // marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
         {
            entity_buf_iter = entity_buffer->buffer;
            for(int i = 0; i < entity_buffer->size; ++i)
            {
               if(entity_buf_iter->entity == collision_data.collided_entity_ptr)
               {
                  entity_buf_iter->collision_check = true;
                  break;
               }
               entity_buf_iter++;
            }
         }
         
         // deals with collision
         switch(collision_data.collision_outcome)
         {
            case STEPPED_SLOPE:
            {
               // cout << "PLAYER STEPPED INTO SLOPE \n";
               player->standing_entity_ptr = collision_data.collided_entity_ptr;
               player->entity_ptr->position.y += collision_data.overlap;
               // @TODO: this is weird, here we are kinda assuming the player is already standing,
               // (because we do not set its state to standing) and this is not that wrong since,
               // above, we already checked for vertical collisions and possibly dealt with... lost track of it. 
               break;
            }
            case BLOCKED_BY_WALL:
            {
               // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
               player->entity_ptr->position -= vec3(
                  collision_data.normal_vec.x, 0, collision_data.normal_vec.y
               ) * collision_data.overlap;
               break;
            }
         }
      }
      else end_collision_checks = true;
   }
}

// @NOTE! : Because we are marking entities in buffer when checked, we should never use 
// multiple CONTROLLER LEVEL calls unless we reset the buffers to the active scene entity list
void run_collision_checks_falling(Player* player, size_t entity_list_size)
{
   // Here, we will check first for vertical intersections to see if player needs to be teleported 
   // to the top of the entity it has collided with. 
   // If so, we deal with it on the spot (teleports player) and then keep checking for other collisions
   // to be resolved once thats done. Horizontal checks come after vertical collisions because players 
   // shouldnt loose the chance to make their jump because we are preventing them from getting stuck first.

   auto entity_buffer = (EntityBuffer*)G_BUFFERS.buffers[0];  
   bool end_collision_checks = false;
   while(!end_collision_checks)
   {
      bool any_collision = false;
      // CHECKS VERTICAL COLLISIONS
      {
         auto entity_iter = entity_buffer->buffer;  // places pointer back to start
         auto collision_data = check_collision_vertical(player, entity_iter, entity_list_size);
         if(collision_data.collided_entity_ptr != NULL)
         {
            // we have collided ladies and gents
            any_collision = true;

            // marks entity in entity buffer as checked so we dont check collisions 
            // for this entity twice (nor infinite loop)
            {
               entity_iter = entity_buffer->buffer;
               for(int i = 0; i < entity_buffer->size; ++i)
               {
                  if(entity_iter->entity == collision_data.collided_entity_ptr)
                  {
                     entity_iter->collision_check = true;
                     break;
                  }
                  entity_iter++;
               }
            }

            // the point of this is to not trigger health check when player
            // hits a wall while falling
            bool trigger_check_was_player_hurt = false;
            
            // deals with collision
            switch(collision_data.collision_outcome)
            {
               case JUMP_FAIL:
               {
                  trigger_check_was_player_hurt = true;
                  // make player "slide" towards edge and fall away from floor
                  std::cout << "FELL FROM EDGE" << "\n";
                  player->standing_entity_ptr = collision_data.collided_entity_ptr;
                  player->entity_ptr->position.y += collision_data.overlap; 
                  
                  // checks if we need to set player's velocity to the opposite direction from where he is coming
                  bool revert_player_movement = false;
                  if(collision_data.normal_vec.x != 0)
                  {
                     float player_movement_sign = player->entity_ptr->velocity.x > 0 ? 1 : -1;
                     revert_player_movement = player_movement_sign == collision_data.normal_vec.x;
                  }
                  else if(collision_data.normal_vec.y != 0)
                  {
                     float player_movement_sign = player->entity_ptr->velocity.z > 0 ? 1 : -1;
                     revert_player_movement = player_movement_sign == collision_data.normal_vec.y;
                  }

                  // makes player move towards OUT of the platform
                  if(revert_player_movement ||
                     (player->entity_ptr->velocity.x == 0 &&
                      player->entity_ptr->velocity.z == 0))
                  {
                     player->entity_ptr->velocity.x = -1 * collision_data.normal_vec.x * player->fall_from_edge_speed;
                     player->entity_ptr->velocity.z = -1 *collision_data.normal_vec.y * player->fall_from_edge_speed;
                  }
                  // makes player fall (combined movement in 3D, player for a moment gets "inside" the platform while he slips)
                  player->entity_ptr->velocity.y = - 1 * player->fall_speed;
                  
                  player->player_state = PLAYER_STATE_FALLING_FROM_EDGE;
                  break;
               }
               case JUMP_SUCCESS:
               {
                  trigger_check_was_player_hurt = true;
                  std::cout << "LANDED" << "\n";
                  // move player to surface, stop player and set him to standing
                  player->standing_entity_ptr = collision_data.collided_entity_ptr;
                  auto height_check = sample_terrain_height_at_player(player->entity_ptr, player->standing_entity_ptr);
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
                        vec3(collision_data.normal_vec.x, 0, collision_data.normal_vec.y)  * collision_data.overlap;

                  // make player slide through the tangent of platform
                  auto tangent_vec = vec2(collision_data.normal_vec.y, collision_data.normal_vec.x);      
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
                  make_player_slide(player, collision_data);
                  break;
               }
               case JUMP_SLIDE_HIGH_INCLINATION:
               {
                  trigger_check_was_player_hurt = true;
                  make_player_slide_fall(player, collision_data);
                  break;
               }
               case JUMP_CEILING:
               {
                  std::cout << "HIT CEILING" << "\n";
                  player->entity_ptr->position.y -= collision_data.overlap + COLLISION_EPSILON; 
                  player->player_state = PLAYER_STATE_FALLING;
                  player->entity_ptr->velocity.y = 0;
                  break; 
               }
            }

            // hurts player if necessary
            if(trigger_check_was_player_hurt)
            {
               float fall_height = player->height_before_fall - player->entity_ptr->position.y;
               cout << "->" << fall_height << "\n";
               if(fall_height >= 3.2)
                  player->lives -= 2;
               else if(fall_height >= 1.8)
                  player->lives -= 1;
            }  
         }
      }

      // CHECKS HORIZONTAL COLLISIONS
      {
         auto entity_iter = entity_buffer->buffer;  // places pointer back to start
         auto collision_data = check_collision_horizontal(player, entity_iter, entity_list_size);
         if(collision_data.collided_entity_ptr != NULL)
         {
            any_collision = true;
            // marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
            {
               entity_iter = entity_buffer->buffer;
               for(int i = 0; i < entity_buffer->size; ++i)
               {
                  if(entity_iter->entity == collision_data.collided_entity_ptr)
                  {
                     entity_iter->collision_check = true;
                     break;
                  }
                  entity_iter++;
               }
            }
            
            // deals with collision
            switch(collision_data.collision_outcome)
            {
               case STEPPED_SLOPE:
               {
                  cout << "PLAYER STEPPED INTO SLOPE \n";
                  player->standing_entity_ptr = collision_data.collided_entity_ptr;
                  player->entity_ptr->position.y += collision_data.overlap;
                  // @TODO: this is weird, here we are kinda assuming the player is already standing,
                  // (because we do not set its state to standing) and this is not that wrong since,
                  // above, we already checked for vertical collisions and possibly dealt with... lost track of it. 
                  break;
               }
               case BLOCKED_BY_WALL:
               {
                  // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
                   player->entity_ptr->position -= vec3(
                     collision_data.normal_vec.x, 0, collision_data.normal_vec.y
                  ) * collision_data.overlap;
                  break;
               }
            }
         }
      }
      if(!any_collision)
      {
         end_collision_checks = true;
      }
   }
}


void make_player_slide(Player* player, CollisionData collision_data)
{
   std::cout << "SLIDING" << "\n";
   player->standing_entity_ptr = collision_data.collided_entity_ptr;
   auto height_check = sample_terrain_height_at_player(player->entity_ptr, player->standing_entity_ptr);
   player->entity_ptr->position.y = height_check.overlap + player->half_height;
   // make player 'snap' to slope
   auto collision_geom = collision_data.collided_entity_ptr->collision_geometry.slope;
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

   pv = player->slide_speed * collision_geom.inclination * collision_geom.tangent;
   player->player_state = PLAYER_STATE_SLIDING;
}

void make_player_slide_fall(Player* player, CollisionData collision_data)
{
   std::cout << "SLIDING" << "\n";
   player->standing_entity_ptr = collision_data.collided_entity_ptr;
   auto height_check = sample_terrain_height_at_player(player->entity_ptr, player->standing_entity_ptr);
   player->entity_ptr->position.y = height_check.overlap + player->half_height;
   // make player 'snap' to slope
   auto collision_geom = collision_data.collided_entity_ptr->collision_geometry.slope;
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
   player->player_state = PLAYER_STATE_SLIDE_FALLING;
}

// ______________________________________
//
//  ENTITY COLLISION CONTROLLER FUNCTIONS
// ______________________________________

CollisionData check_collision_horizontal(Player* player, EntityBufferElement* entity_iterator, size_t entity_list_size) 
{
   CollisionData return_cd; 
   // this serves only to enable us to check for standing_entity_ptr, otherwise its NULL and we get an exception
   bool player_qualifies_as_standing = 
      player->player_state == PLAYER_STATE_STANDING || 
      player->player_state == PLAYER_STATE_SLIDING  ||
      player->player_state == PLAYER_STATE_SLIDE_FALLING;

   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;
	   float biggest_overlap = -1;
      Collision c;
      bool set_collided_entity = false;     
	   if (entity_iterator->collision_check == false && 
         !(player_qualifies_as_standing && player->standing_entity_ptr == entity))
      {    
         // AABB
         if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX &&
            intersects_vertically_with_aabb(entity, player))
         {
            c = get_horizontal_overlap_with_player(entity, player);

            if(c.is_collided && c.overlap >= 0 && c.overlap > biggest_overlap)
            {
               cout << "collided with " << entity->name << "\n";
               return_cd.collision_outcome = BLOCKED_BY_WALL;
               set_collided_entity = true;
            }
         }

         // ALIGNED SLOPE
         else if (entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE &&
                  intersects_vertically_with_slope(entity, player->entity_ptr))
         {
            c = get_horizontal_overlap_with_player(entity, player);
            if(c.is_collided && c.overlap > biggest_overlap)
            {
               auto col_geometry = entity->collision_geometry.slope;
               auto slope_2d_tangent = glm::normalize(vec2(col_geometry.tangent.x, col_geometry.tangent.z));

               if(player->player_state == PLAYER_STATE_STANDING &&
                  c.overlap == 0)
               {
                  set_collided_entity = true;
                  return_cd.collision_outcome = STEPPED_SLOPE;
                  // @WORKAROUND
                  float v_overlap = get_vertical_overlap_player_vs_slope(entity, player);
                  c.overlap = v_overlap;
               }

               else if(player->player_state == PLAYER_STATE_STANDING &&
                  c.overlap > 0 &&  // this means player is not INSIDE entity (player centroid)
                  compare_vec2(c.normal_vec, -1.0f * slope_2d_tangent))
               {
                  if(col_geometry.inclination > SLIDE_MIN_ANGLE)
                  {
                     set_collided_entity = true;
                     return_cd.collision_outcome = BLOCKED_BY_WALL;
                  }
               }
               else if(c.overlap > 0)
               {
                  set_collided_entity = true;
                  return_cd.collision_outcome = BLOCKED_BY_WALL;
               }
            }
         }

         // set current entity as collided one
         if(set_collided_entity)
         {
            // cout << "horizontal collision with '" << entity->name << "'\n";
            biggest_overlap = c.overlap;

            return_cd.is_collided = true;
            return_cd.collided_entity_ptr = entity;
            return_cd.overlap = c.overlap;
            return_cd.normal_vec = c.normal_vec;
         }
      }
      entity_iterator++;
   }
   return return_cd;
}


CollisionData check_collision_vertical(Player* player, EntityBufferElement* entity_iterator, size_t entity_list_size)
{
   CollisionData return_cd; 
   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;
	   float biggest_overlap = -1;
	   if (entity_iterator->collision_check == false)
      {   
         // PERFORMS OVERLAP TESTING
         float vertical_overlap = -1;
         Collision v_overlap_collision; // for ceilling hit detection
         Collision horizontal_check;
         {
            if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX && intersects_vertically_with_aabb(entity, player))
            {
               v_overlap_collision = get_vertical_overlap_player_vs_aabb(entity, player->entity_ptr);
               vertical_overlap = v_overlap_collision.overlap;
               horizontal_check = get_horizontal_overlap_with_player(entity, player);
            }
            else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE && intersects_vertically_with_slope(entity, player->entity_ptr))
            {
               vertical_overlap = get_vertical_overlap_player_vs_slope(entity, player);
               horizontal_check = get_horizontal_overlap_with_player(entity, player);   
            }
         }
         // CHECKS IF ANYTHING WORHTWHILE HAPPENED
         if(horizontal_check.is_collided && vertical_overlap >= 0 && vertical_overlap > biggest_overlap)
         {
            biggest_overlap = vertical_overlap;
            return_cd.collided_entity_ptr = entity;

            // PARTICULAR CHECKS FOR SLOPES (PLAYER LIES INSIDE IT)
            if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE &&
               horizontal_check.overlap == 0)
            {
               auto col_geometry = entity->collision_geometry.slope;
               if(col_geometry.inclination > SLIDE_MAX_ANGLE)
               {
                  return_cd.overlap = vertical_overlap;
                  return_cd.collision_outcome = JUMP_SLIDE_HIGH_INCLINATION;
               }
               else if(col_geometry.inclination > SLIDE_MIN_ANGLE)
               {
                  return_cd.overlap = vertical_overlap;
                  return_cd.collision_outcome = JUMP_SLIDE;
               }
               else
               {
                  return_cd.overlap = vertical_overlap;
                  return_cd.collision_outcome = JUMP_SUCCESS;
               }
            }

            // player fell right inside geometry
            else if(horizontal_check.overlap == 0 && 
               v_overlap_collision.normal_vec.y != -1)
            {
               return_cd.overlap = vertical_overlap;
               return_cd.collision_outcome = JUMP_SUCCESS;
            }

            // player jumped and hit the ceiling
            else if(v_overlap_collision.normal_vec.y == -1 &&
               player->prior_position.y < player->entity_ptr->position.y &&
               vertical_overlap < 0.03)
            {
               return_cd.overlap = vertical_overlap;
               return_cd.normal_vec = v_overlap_collision.normal_vec;
               return_cd.collision_outcome = JUMP_CEILING;
            }

            // player intersected with wall too much below standing area (hit wall)
            else if(vertical_overlap > 0.001) 
            {
               return_cd.overlap = horizontal_check.overlap;
               return_cd.normal_vec = horizontal_check.normal_vec;
               return_cd.collision_outcome = JUMP_FACE_FLAT;
            }

            // player did not get half body inside platform (didnt make the jump)
            else if(horizontal_check.overlap < player->radius)
            {
               return_cd.overlap = vertical_overlap;
               return_cd.normal_vec = horizontal_check.normal_vec;
               return_cd.collision_outcome = JUMP_FAIL;
            }

            // got at least half body inside platform (made the jump)
            else
            {
               return_cd.overlap = vertical_overlap;
               return_cd.collision_outcome = JUMP_SUCCESS;
            }
         }
      }
      entity_iterator++;
   }
   return return_cd;
}


//@todo: refactor this into game mode input handle and editor mode input handle
void handle_input_flags(InputFlags flags, Player* &player)
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
      G_FRAME_INFO.time_step = 0.05;
   }
   if(pressed_once(flags, KEY_2))
   {
      G_FRAME_INFO.time_step = 0.1;
   }
   if(pressed_once(flags, KEY_3))
   {
      G_FRAME_INFO.time_step = 1.0;
   }
   if(pressed_once(flags, KEY_4))
   {
      G_FRAME_INFO.time_step = 0.3;
   }
   if(pressed_once(flags, KEY_5))
   {
      G_FRAME_INFO.time_step = 2.0;
   }
   if(flags.key_press & KEY_K)
   {
      load_player_attributes_from_file(G_SCENE_INFO.scene_name, player);
      player->lives = 2;
   }
   if(pressed_once(flags, KEY_F))
   {
      if(PROGRAM_MODE.current == EDITOR_MODE)
      {
         PROGRAM_MODE.last = PROGRAM_MODE.current;
         PROGRAM_MODE.current = GAME_MODE;
         G_SCENE_INFO.camera = G_SCENE_INFO.views[1];
         player->entity_ptr->render_me = false;
         glfwSetInputMode(G_DISPLAY_INFO.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
         Editor::end_frame();
      }
      else if(PROGRAM_MODE.current == GAME_MODE)
      {
         PROGRAM_MODE.last = PROGRAM_MODE.current;
         PROGRAM_MODE.current = EDITOR_MODE;
         G_SCENE_INFO.camera = G_SCENE_INFO.views[0];
         player->entity_ptr->render_me = true;
         glfwSetInputMode(G_DISPLAY_INFO.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }
   }
   if(flags.key_press & KEY_ESC && flags.key_press & KEY_LEFT_SHIFT)
   {
       glfwSetWindowShouldClose(G_DISPLAY_INFO.window, true);
   }
}

void game_handle_input_flags(InputFlags flags, Player* &player)
{

   if(player->player_state == PLAYER_STATE_STANDING)
   {
      // resets velocity
      player->entity_ptr->velocity = vec3(0); 

      if(flags.key_press & KEY_W)
      {
         player->entity_ptr->velocity += vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
      }
      if(flags.key_press & KEY_A)
      {
         vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
         player->entity_ptr->velocity -= vec3(onwards_vector.x, 0, onwards_vector.z);
      }
      if(flags.key_press & KEY_S)
      {
         player->entity_ptr->velocity -= vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
      }
      if(flags.key_press & KEY_D)
      {
         vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
         player->entity_ptr->velocity += vec3(onwards_vector.x, 0, onwards_vector.z);
      }
      // because above we sum all combos of keys pressed, here we normalize the direction and give the movement intensity
      if(glm::length2(player->entity_ptr->velocity) > 0)
      {
         float player_frame_speed = player->speed;
         if(flags.key_press & KEY_LEFT_SHIFT)  // PLAYER DASH
            player_frame_speed *= 2;

         player->entity_ptr->velocity = player_frame_speed * glm::normalize(player->entity_ptr->velocity);
      }
      if (flags.key_press & KEY_SPACE) 
      {
         player->player_state = PLAYER_STATE_JUMPING;
         player->height_before_fall = player->entity_ptr->position.y;
         player->entity_ptr->velocity.y = player->jump_initial_speed;
      }

      // update camera with player position
      G_SCENE_INFO.camera->Position = player->entity_ptr->position;
      G_SCENE_INFO.camera->Position.y +=  player->half_height * 2.0 / 3.0;
   }
   else if(player->player_state == PLAYER_STATE_SLIDING)
   {
      auto collision_geom = player->standing_entity_ptr->collision_geometry.slope;
      player->entity_ptr->velocity = player->slide_speed * collision_geom.tangent;

      if (flags.key_press & KEY_A)
      {
         float dot_product = glm::dot(collision_geom.tangent, G_SCENE_INFO.camera->Front);
         float angle = -12.0f;
         if (dot_product < 0)
         {
            angle *= -1;
         }

         auto bitangent = glm::cross(collision_geom.tangent, G_SCENE_INFO.camera->Up);
         auto normal = glm::cross(bitangent, collision_geom.tangent);
         auto temp_vec = glm::rotate(player->entity_ptr->velocity, angle, normal);
         player->entity_ptr->velocity.x = temp_vec.x;
         player->entity_ptr->velocity.z = temp_vec.z;
      }
      if (flags.key_press & KEY_D)
      {
         float dot_product = glm::dot(collision_geom.tangent, G_SCENE_INFO.camera->Front);
         float angle = 12.0f;
         if (dot_product < 0)
         {
            angle *= -1;
         }

         auto bitangent = glm::cross(collision_geom.tangent, G_SCENE_INFO.camera->Up);
         auto normal = glm::cross(bitangent, collision_geom.tangent);
         auto temp_vec = glm::rotate(player->entity_ptr->velocity, angle, normal);
         player->entity_ptr->velocity.x = temp_vec.x;
         player->entity_ptr->velocity.z = temp_vec.z;
      }
      if (flags.key_press & KEY_SPACE)
      {
            player->player_state = PLAYER_STATE_JUMPING;
            auto collision_geom = player->standing_entity_ptr->collision_geometry.slope;
            float x = collision_geom.normal.x > 0 ? 1 : collision_geom.normal.x == 0 ? 0 : -1;
            float z = collision_geom.normal.z > 0 ? 1 : collision_geom.normal.z == 0 ? 0 : -1;
            auto jump_vec = glm::normalize(vec3(x, 1, z));
            player->entity_ptr->velocity = player->jump_initial_speed * jump_vec;
      }
   }
}

void reset_input_flags(InputFlags flags)
{
   // here we record a history for if keys were last pressed or released, so to enable smooth toggle
   G_INPUT_INFO.key_state |= flags.key_press;
   G_INPUT_INFO.key_state &= ~(flags.key_release); 
}

void player_death_handler(Player* &player)
{
   bool loaded = load_player_attributes_from_file(G_SCENE_INFO.scene_name, player);
   player->lives = 2;
}