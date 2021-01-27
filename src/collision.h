

struct CollisionData{
   Entity* collided_entity_ptr;
   float distance_from_position;
};


float check_collision_aligned_cylinder_vs_aligned_box(Entity* entity, glm::vec3 position, glm::vec3 velocity);
CollisionData check_player_collision_with_scene(Entity* player, Entity* entity, size_t entity_list_size); 


CollisionData check_player_collision_with_scene(Entity* player, Entity* entity, size_t entity_list_size) 
{

   Entity* collided_first_with_player = NULL;
   float distance_to_nearest_collision = MAX_FLOAT;
   for(int i = 0; i < entity_list_size; i++) 
   {
      float distance_to_collision = MAX_FLOAT;
      switch(entity->collision_geometry_type)
      {
         case COLLISION_ALIGNED_BOX:
         distance_to_collision =
            check_collision_aligned_cylinder_vs_aligned_box(entity->collision_geometry_prt, entity->position, entity->velocity);
         break;
      }

      if(distance_to_collision < distance_to_nearest_collision)
      {
         distance_to_nearest_collision = distance_to_collision;
         collided_first_with_player = entity;
      }

      entity++;
      i++;
   }

   CollisionData cd { collided_fisrt_with_player, distance_to_nearest_collision };
   return cd;
}


float check_collision_aligned_cylinder_vs_aligned_box(Entity* entity, glm::vec3 position, glm::vec3 velocity)
{
   // takes player's position and velocity this frame and next frame ( velocity is in units/second, take
   // the delta_time from frame and calculate position in units of next frame)

   // then subdivide that space of motion into smaller steps

   // now it becomes a matter of strategy. How to check for player collision against boxes?

   // check for each subdivision, either moving forward in time from start until finish (o(n) always) or start at 
   // the end and come back ((log n) worst case, but doesnt really make much sense to me)

   // etc. 
}