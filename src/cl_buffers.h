
// ----------------------------
// > UPDATE PLAYER WORLD CELLS
// ----------------------------

bool CL_update_player_world_cells(Player* player)
{
   // update player world cells
   auto offset1               = vec3{-1.0f * player->radius, -1.0f * player->half_height, -1.0f * player->radius};
   auto offset2               = vec3{player->radius, 0, player->radius};
   auto update_cells          = World.update_entity_world_cells(player->entity_ptr, offset1, offset2);
   if(update_cells.status == CellUpdate_OK)
   {
      return update_cells.entity_changed_cell;
   }
   else
   { 
      cout << update_cells.message << "\n";
      return false;
   }
}


// ------------------------------
// > COLLISION BUFFER FUNCTIONS
// ------------------------------

void CL_recompute_collision_buffer_entities(Player* player)
{
   // copies collision-check-relevant entity ptrs to a buffer
   // with metadata about the collision check for the entity
   auto collision_buffer      = G_BUFFERS.entity_buffer->buffer;
   int entity_count           = 0;
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
            collision_buffer[entity_count].entity           = entity;
            collision_buffer[entity_count].collision_check  = false;
            entity_count++;
            if(entity_count > COLLISION_BUFFER_CAPACITY) assert(false);
         }
      }
   }

   G_BUFFERS.entity_buffer->size = entity_count;
}


void CL_reset_collision_buffer_checks()
{
   for(int i = 0; i < G_BUFFERS.entity_buffer->size; i++)
      G_BUFFERS.entity_buffer->buffer[i].collision_check = false;
}


void CL_mark_entity_checked(Entity* entity)
{
   // marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
   auto entity_buffer         = G_BUFFERS.entity_buffer;
   auto entity_element        = entity_buffer->buffer;
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