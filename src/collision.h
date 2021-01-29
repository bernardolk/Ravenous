

struct CollisionData{
   Entity* collided_entity_ptr;
   float distance_from_position;
   bool collision;
};


float check_collision_aligned_cylinder_vs_aligned_box(Entity* entity, Entity* player);
CollisionData check_player_collision_with_scene(Entity* player, Entity* entity, size_t entity_list_size);
CollisionData sample_terrain_height_below_player(Entity* player, Entity* entity); 


CollisionData check_player_collision_with_scene(Entity* player, Entity** entity_iterator, size_t entity_list_size) 
{

   Entity* collided_first_with_player = NULL;
   float distance_to_nearest_collision = MAX_FLOAT;
   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = *entity_iterator;
	   float distance_to_collision = MAX_FLOAT;

	   if (entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
	   {
         distance_to_collision =
            check_collision_aligned_cylinder_vs_aligned_box(entity, player);
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
   // this method is pretty stupid and gimmecky

   // it only works fiven prior knowledge of objects orientation (box)
   glm::vec3 player_next_frame_position = player->position + player->velocity * G_FRAME_INFO.delta_time;

   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   float player_bottom = player->position.y - player_collision_geometry->half_length;
   float player_top = player->position.y + player_collision_geometry->half_length;

   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
   float box_top = entity->position.y + box_collision_geometry.length_y;
   float box_bottom = entity->position.y;

   //box boundaries
   float box_x0 = entity->position.x - box_collision_geometry.length_x;
   float box_z0 = entity->position.z;
   float box_x1 = entity->position.x;
   float box_z1 = entity->position.z + box_collision_geometry.length_z;

   // for debug, draw boundaries
   if(entity->name == "Platform 1")
   {
      Entity* line = find_entity_in_scene(G_SCENE_INFO.active_scene, "LINE1");

      line->model->mesh.vertices[0].position = glm::vec3(box_x0, box_top + 0.2, box_z0);
      line->model->mesh.vertices[1].position = glm::vec3(box_x1, box_top + 0.2, box_z0);
      line->model->mesh.vertices[2].position = glm::vec3(box_x1, box_top + 0.2, box_z1);
      line->model->mesh.vertices[3].position = glm::vec3(box_x0, box_top + 0.2, box_z1);

      glBindVertexArray(line->model->gl_data.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, line->model->gl_data.VBO);
      glBufferData(GL_ARRAY_BUFFER, line->model->mesh.vertices.size() * sizeof(Vertex), &(line->model->mesh.vertices[0]), GL_STATIC_DRAW);
   }
   else if(entity->name == "Platform 2")
   {
      Entity* line = find_entity_in_scene(G_SCENE_INFO.active_scene, "LINE2");

      line->model->mesh.vertices[0].position = glm::vec3(box_x0, box_top + 0.2, box_z0);
      line->model->mesh.vertices[1].position = glm::vec3(box_x1, box_top + 0.2, box_z0);
      line->model->mesh.vertices[2].position = glm::vec3(box_x1, box_top + 0.2, box_z1);
      line->model->mesh.vertices[3].position = glm::vec3(box_x0, box_top + 0.2, box_z1);

      glBindVertexArray(line->model->gl_data.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, line->model->gl_data.VBO);
      glBufferData(GL_ARRAY_BUFFER, line->model->mesh.vertices.size() * sizeof(Vertex), &(line->model->mesh.vertices[0]), GL_STATIC_DRAW);
   }

   // only takes account for player falling (player above going down)
   if(player_bottom <= box_top) 
   {
      float player_x = player->position.x;
      float player_z = player->position.z;

      
      if(box_x0 <= player_x && box_x1 >= player_x && box_z0 <= player_z && box_z1 >= player_z)
      {
         return player->position.y - box_top;
      }
   }

   return -1;
}


CollisionData sample_terrain_height_below_player(Entity* player, Entity* entity)
{
   // simplified version of full algorithm

   // returns -1 if player not standing on entity

   // here, we assume that we KNOW in what entity the player is standing on.
   // therefore, we can get the 'terrain' entity directly, and scan it's walkable
   // surface geometry to see what is the height (y coordinate) in the player's
   // x-z coordinates.
   // in the FULL VERSION we will need to scan all triangles in the walkable surface mesh
   // to find which triangle represents the surface (3 points) to interpolate the height.
   // We can find the correct triangle by getting the highest x and z coordinates from the
   // three vertices and checking against the player's coordinates (is the player inside those?)
   // then we use the surface equation to get the height at the exact coordinate.

   // in the SIMPLIFIED VERSION, we know we have a flat axis-aligned platform. Just check the
   // boundaries to see if player crossed or not.

   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
   float box_top = entity->position.y + box_collision_geometry.length_y;
   float box_bottom = entity->position.y;

   //platform boundaries
   float box_x0 = entity->position.x - box_collision_geometry.length_x;
   float box_z0 = entity->position.z;
   float box_x1 = entity->position.x;
   float box_z1 = entity->position.z + box_collision_geometry.length_z;

   float player_x = player->position.x;
   float player_z = player->position.z;

   CollisionData cd;

   // check if player is inside terrain box
   if(box_x0 <= player_x && box_x1 >= player_x && box_z0 <= player_z && box_z1 >= player_z)
   {
      cd.distance_from_position = box_top;
      cd.collision = true;
      return cd;
   }
   else {
      cd.collision = false;
      return cd;
   }
}
