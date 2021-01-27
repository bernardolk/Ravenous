

struct CollisionData{
   Entity* collided_entity_ptr;
   float distance_from_position;
};


float check_collision_aligned_cylinder_vs_aligned_box(Entity* entity, Entity* player);
CollisionData check_player_collision_with_scene(Entity* player, Entity* entity, size_t entity_list_size); 


CollisionData check_player_collision_with_scene(Entity* player, Entity** entity_iterator, size_t entity_list_size) 
{

   Entity* collided_first_with_player = NULL;
   float distance_to_nearest_collision = MAX_FLOAT;
   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = *entity_iterator;
	   float distance_to_collision = MAX_FLOAT;
	   /* switch(entity->collision_geometry_type)
		{*/
		//case COLLISION_ALIGNED_BOX:
	   if (entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
	   {
         distance_to_collision =
            check_collision_aligned_cylinder_vs_aligned_box(entity, player);
        // break;
      }

      if(distance_to_collision > 0 && distance_to_collision < distance_to_nearest_collision)
      {
         distance_to_nearest_collision = distance_to_collision;
         collided_first_with_player = entity;
      }

      entity_iterator++;
   }

   CollisionData cd { collided_first_with_player, distance_to_nearest_collision };
   return cd;
}


float check_collision_aligned_cylinder_vs_aligned_box(Entity* entity, Entity* player)
{
   glm::vec3 player_next_frame_position = player->position + player->velocity * G_FRAME_INFO.delta_time;
   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   float player_bottom = player->position.y - player_collision_geometry->half_length;
   float player_top = player->position.y + player_collision_geometry->half_length;
   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
   float box_top = entity->position.y + box_collision_geometry.length_y;
   float box_bottom = entity->position.y;

   if(player_bottom <= box_top) 
   {
      float player_x = player->position.x;
      float player_z = player->position.z;

      float box_x0 = entity->position.x;
      float box_z0 = entity->position.z;
      float box_x1 = entity->position.x + box_collision_geometry.length_x;
      float box_z1 = entity->position.z + box_collision_geometry.length_z;
      if(box_x0 <= player_x && box_x1 >= player_x && box_z0 <= player_z && box_z1 >= player_z)
      {
         return 3;
      }
   }

   return -1;
}