

struct CollisionData{
   Entity* collided_entity_ptr;
   float distance_from_position;
   bool collision;
};


float check_collision_aligned_cylinder_vs_aligned_box(Entity* entity, Entity* player);
CollisionData check_player_collision_with_scene(Entity* player, Entity* entity, size_t entity_list_size);
CollisionData sample_terrain_height_below_player(Entity* player, Entity* entity); 
bool check_2D_collision_circle_and_aligned_square(Entity* circle, Entity* square);
float squared_distance_between_point_and_line(float x1, float y1, float x2, float y2, float x0, float y0);
float squared_minimum_distance(glm::vec2 v, glm::vec2 w, glm::vec2 p);


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


float squared_distance_between_point_and_line(float x1, float y1, float x2, float y2, float x0, float y0)
{
   // 0 is point, 1 and 2 define a line

   float numerator = (x2 - x1) * (y1 - y0) - (x1 - x0) * (y2 - y1);
   numerator = numerator * numerator;
   float denominator = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
   assert(denominator > 0);
   return numerator / denominator;
}


bool check_2D_collision_circle_and_aligned_square(Entity* circle, Entity* square)
{
   // Note: this will only check if there are edges of the quare inside the circle.
   // If the circle is completely inside the square, this will not work!

   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) square->collision_geometry_ptr);

   //square vertices
   float square_x0 = square->position.x - box_collision_geometry.length_x;
   float square_z0 = square->position.z;
   float square_x1 = square->position.x;
   float square_z1 = square->position.z + box_collision_geometry.length_z;

   float player_x = circle->position.x;
   float player_z = circle->position.z;

   float d_1 = squared_minimum_distance(glm::vec2(square_x0, square_z0), glm::vec2(square_x1, square_z0), glm::vec2(player_x, player_z));
   float d_2 = squared_minimum_distance(glm::vec2(square_x1, square_z0), glm::vec2(square_x1, square_z1), glm::vec2(player_x, player_z));
   float d_3 = squared_minimum_distance(glm::vec2(square_x1, square_z1), glm::vec2(square_x0, square_z1), glm::vec2(player_x, player_z));
   float d_4 = squared_minimum_distance(glm::vec2(square_x0, square_z1), glm::vec2(square_x0, square_z0), glm::vec2(player_x, player_z));

   std::cout << "player: (" << player_x << "," << player_z << ") ; d1: " << d_1 << ", d2: " << d_2 << ", d3: " << d_3 << ", d4: " << d_4 << "\n";

   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) circle->collision_geometry_ptr;
   float p_radius2 = player_collision_geometry->radius * player_collision_geometry->radius;
   std::cout << "radius2: " << p_radius2 << "\n";
   if(d_1 <= p_radius2)
   {
      std::cout << "intersecting with 1 !" << "\n";
      return true;
   }
   else if(d_2 <= p_radius2)
   {
      std::cout << "intersecting with 2 !" << "\n";
      return true;
   }
   else if(d_3 <= p_radius2)
   {
      std::cout << "intersecting with 3 !" << "\n";
      return true;
   }
   else if(d_4 <= p_radius2)
   {
      std::cout << "intersecting with 4 !" << "\n";
      return true;
   }

   return false;
}


float squared_minimum_distance(glm::vec2 v, glm::vec2 w, glm::vec2 p) 
{
  // Return minimum distance between line segment vw and point p
  float l2 = glm::length2(v - w);  // i.e. |w-v|^2 -  avoid a sqrt
  if (l2 == 0.0) return glm::distance(p,v);   // v == w case
  float dot = glm::dot(p - v, w - v) / l2;
  float min = 1 > dot ? dot : 1;
  float t = 0 > min ? 0 : min;
  glm::vec2 projection = v + t * (w - v);  // Projection falls on the segment
  float d = glm::distance(p, projection);
  return d * d;
}


