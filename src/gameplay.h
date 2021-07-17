void handle_common_input(InputFlags flags, Player* &player);
void handle_movement_input(InputFlags flags, Player* &player, ProgramModeEnum pm);
void run_collision_checks_standing(Player* player);
void run_collision_checks_falling(Player* player);
void mark_entity_checked(Entity* entity);
void resolve_collision(CollisionData collision, Player* player);
void check_for_floor_transitions(Player* player);
void check_trigger_interaction(Player* player);
bool update_player_world_cells(Player* player);
void recompute_collision_buffer_entities(Player* player);
void reset_collision_buffer_checks();
void make_player_slide(Player* player, Entity* ramp, bool slide_fall = false);
void make_player_jump_from_slope(Player* player);

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


// ----------------
// COLLISION CELLS
// ----------------
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
 
// -------
// ACTION
// -------
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

// -------------------------------------
// SCENE COLLISION CONTROLLER FUNCTIONS
// -------------------------------------
//    iterative collision detection
// -------------------------------------

void run_collision_checks_standing(Player* player)
{
   auto entity_buffer = G_BUFFERS.entity_buffer;
   int c = -1;
   while(true)
   {
      c++;
      auto buffer = entity_buffer->buffer;  // places pointer back to start
      CollisionData collision_data = check_collision_horizontal(player, buffer, entity_buffer->size);
      if(collision_data.collided_entity_ptr != NULL)
      {
         mark_entity_checked(collision_data.collided_entity_ptr);
         log_collision(collision_data, c);
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
   int c = -1;
   while(true)
   {
      c++;
      bool any_collision = false;
      // CHECKS VERTICAL COLLISIONS
      {
         auto buffer = entity_buffer->buffer;  // places pointer back to start
         auto v_collision_data = check_collision_vertical(player, buffer, entity_buffer->size);
         if(v_collision_data.collided_entity_ptr != NULL)
         {
            any_collision = true;
            mark_entity_checked(v_collision_data.collided_entity_ptr);
            log_collision(v_collision_data, c);
            resolve_collision(v_collision_data, player);
         }

         buffer = entity_buffer->buffer;  // places pointer back to start
         auto h_collision_data = check_collision_horizontal(player, buffer, entity_buffer->size);
         if(h_collision_data.collided_entity_ptr != NULL)
         {
            any_collision = true;
            mark_entity_checked(h_collision_data.collided_entity_ptr);
            log_collision(h_collision_data, c);
            resolve_collision(h_collision_data, player);
         }

         if(!any_collision) break;
      }
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
         // conditional animation: if falling from jump, land, else, land from fall
         if(player->half_height < P_HALF_HEIGHT)
            player->anim_state = P_ANIM_LANDING;
         else
            player->anim_state = P_ANIM_LANDING_FALL;
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