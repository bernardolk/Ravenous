void CL_mark_entity_checked                        (Entity* entity);
void CL_run_collision_checks_standing              (Player* player);
void CL_run_collision_checks_falling               (Player* player);
void CL_resolve_collision                          (EntitiesCollision collision, Player* player);
bool CL_update_player_world_cells                  (Player* player);
void CL_recompute_collision_buffer_entities        (Player* player);
void CL_mark_entity_checked                        (Entity* entity);
void CL_reset_collision_buffer_checks              ();


// ----------------------------------------
// > SCENE COLLISION CONTROLLER FUNCTIONS
// ----------------------------------------

void CL_run_collision_checks_standing(Player* player)
{
   // iterative collision detection
   auto entity_buffer = G_BUFFERS.entity_buffer;
   int c = -1;
   while(true)
   {
      c++;
      auto buffer                          = entity_buffer->buffer;            // places pointer back to start
      EntitiesCollision collision_data     = CL_check_collision_horizontal(player, buffer, entity_buffer->size);

      if(collision_data.collided_entity_ptr != NULL)
      {
         CL_mark_entity_checked(collision_data.collided_entity_ptr);
         CL_log_collision(collision_data, c);
         CL_resolve_collision(collision_data, player);
      }
      else break;
   }
}


void CL_run_collision_checks_falling(Player* player)
{
   // Here, we will check first for vertical intersections to see if player needs to be teleported 
   // to the top of the entity it has collided with. 
   // If so, we deal with it on the spot (teleports player) and then keep checking for other collisions
   // to be resolved once thats done. Horizontal checks come after vertical collisions because players 
   // shouldnt loose the chance to make their jump because we are preventing them from getting stuck first.

   auto entity_buffer  = G_BUFFERS.entity_buffer;
   int c               = -1;
   while(true)
   {
      c++;
      bool any_collision = false;
      // CHECKS VERTICAL COLLISIONS
      {
         auto buffer            = entity_buffer->buffer;  // places pointer back to start
         auto v_collision_data  = CL_check_collision_vertical(player, buffer, entity_buffer->size);
         
         if(v_collision_data.collided_entity_ptr != NULL)
         {
            any_collision = true;
            CL_mark_entity_checked(v_collision_data.collided_entity_ptr);
            CL_log_collision(v_collision_data, c);
            CL_resolve_collision(v_collision_data, player);
         }

         buffer                 = entity_buffer->buffer;  // places pointer back to start
         auto h_collision_data  = CL_check_collision_horizontal(player, buffer, entity_buffer->size);
         if(h_collision_data.collided_entity_ptr != NULL)
         {
            any_collision = true;
            CL_mark_entity_checked(h_collision_data.collided_entity_ptr);
            CL_log_collision(h_collision_data, c);
            CL_resolve_collision(h_collision_data, player);
         }

         if(!any_collision) break;
      }
   }
}
