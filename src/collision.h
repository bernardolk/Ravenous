#include <algorithm>

struct CollisionData{
   bool is_collided = false;
   Entity* collided_entity_ptr;
   float vertical_overlap;
   float horizontal_overlap;
};

struct Collision {
   bool is_collided = false;
   float overlap;
};


Collision get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player);
CollisionData check_player_collision_with_scene(Entity* player, Entity** entity_iterator, size_t entity_list_size, std::string unless_its_this_one); 
CollisionData sample_terrain_height_below_player(Entity* player, Entity* entity); 
bool check_2D_collision_circle_and_aligned_square(Entity* circle, Entity* square);
float squared_distance_between_point_and_line(float x1, float y1, float x2, float y2, float x0, float y0);
float squared_minimum_distance(glm::vec2 v, glm::vec2 w, glm::vec2 p);
glm::vec3 get_nearest_edge(Entity* point, Entity* square);
Collision get_horizontal_overlap_player_aabb(Entity* entity, Entity* player);

CollisionData 
check_player_collision_with_scene(Entity* player, Entity** entity_iterator, size_t entity_list_size, 
                                  std::string unless_its_this_one = "") 
{

   Entity* collided_first_with_player = NULL;
   float vertical = MAX_FLOAT;
   float horizontal = MAX_FLOAT;
   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = *entity_iterator;
	   float distance_to_collision = MAX_FLOAT;
      Collision c;
	   if (entity->collision_geometry_type == COLLISION_ALIGNED_BOX && entity->name != unless_its_this_one)
	   {
         // test vertical collisions (player falling or standing, doesnt matter)
         c = get_vertical_overlap_player_vs_aabb(entity, player);
         if(c.is_collided && c.overlap < vertical)
         {
            vertical = c.overlap;
            collided_first_with_player = entity;

            // this is not how its supposed to be, but since we test for vertical collision (meaning, test
            // for clamping in y and also x-z overlap), we can use it as filter for the next check
            // the next check will calculate x-z overlap and will only be used, for now, when player is standing
            // to move him away from walls
            c = get_horizontal_overlap_player_aabb(entity, player);
            if(c.is_collided)
            {
               horizontal = c.overlap;
            }
         }

      }

      entity_iterator++;
   }

   CollisionData cd;  
   cd.collided_entity_ptr = collided_first_with_player;
   cd.vertical_overlap = vertical;
   cd.horizontal_overlap = horizontal;
   return cd;
}

Collision get_horizontal_overlap_player_aabb(Entity* entity, Entity* player)
{
   //box boundaries
   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
   float box_x0 = entity->position.x;
   float box_x1 = entity->position.x + box_collision_geometry.length_x;
   float box_z0 = entity->position.z;
   float box_z1 = entity->position.z + box_collision_geometry.length_z;

   float player_x = player->position.x;
   float player_z = player->position.z;

   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   float p_radius2 = player_collision_geometry->radius * player_collision_geometry->radius;

   float nx = std::min(box_x0, std::max(box_x1, player_x));
   float nz = std::min(box_z0, std::max(box_z1, player_z));
   float d2 = nx * nx + nz * nz;
   
   Collision c;
   if(p_radius2 > d2)
   {   
      c.is_collided = true;
      c.overlap = p_radius2 - d2;
   }  // else returns false...

   return c;
}


Collision get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player)
{
   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   float player_bottom = player->position.y - player_collision_geometry->half_length;
   float player_top = player->position.y + player_collision_geometry->half_length;

   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
   float box_top = entity->position.y + box_collision_geometry.length_y;
   float box_bottom = entity->position.y;

   //box boundaries
   float box_x0 = entity->position.x;
   float box_x1 = entity->position.x + box_collision_geometry.length_x;
   float box_z0 = entity->position.z;
   float box_z1 = entity->position.z + box_collision_geometry.length_z;

   // check player height comparing to box
   if(player_bottom <= box_top && player_top >= box_bottom) 
   {
      float player_x = player->position.x;
      float player_z = player->position.z;

      // compute nearest dist (2d) vector from player centroid to box
      float nx = std::max(box_x0, std::min(box_x1, player_x));
      float nz = std::max(box_z0, std::min(box_z1, player_z));
      float d2 = nx * nx + nz * nz;
      float p_radius2 = player_collision_geometry->radius * player_collision_geometry->radius;

      // check if player centroid lies inside the box      
      if(
         (box_x0 <= player_x && box_x1 >= player_x && box_z0 <= player_z && box_z1 >= player_z) )
            //|| (p_radius2 > d2))
      {
         float overlap = player->position.y - box_top;
         float overlap_norm = overlap < 0 ? overlap * -1: overlap;
         Collision c;
         c.is_collided = true;
         c.overlap = overlap_norm;
         return c;
      }
   }

   return Collision {false};
}




CollisionData sample_terrain_height_below_player(Entity* player, Entity* entity)
{
   // simplified version of full algorithm

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
   float box_x0 = entity->position.x;
   float box_x1 = entity->position.x + box_collision_geometry.length_x;
   float box_z0 = entity->position.z;
   float box_z1 = entity->position.z + box_collision_geometry.length_z;

   float player_x = player->position.x;
   float player_z = player->position.z;

   CollisionData cd;
   // check if player is inside terrain box
   if(box_x0 <= player_x && box_x1 >= player_x && box_z0 <= player_z && box_z1 >= player_z)
   {
      cd.vertical_overlap = box_top;
      cd.is_collided = true;
   }
   return cd;
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

   //std::cout << "player: (" << player_x << "," << player_z << ") ; d1: " << d_1 << ", d2: " << d_2 << ", d3: " << d_3 << ", d4: " << d_4 << "\n";

   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) circle->collision_geometry_ptr;
   float p_radius2 = player_collision_geometry->radius * player_collision_geometry->radius;
   std::cout << "radius2: " << p_radius2 << "\n";
   if(d_1 <= p_radius2 || d_2 <= p_radius2 || d_3 <= p_radius2 || d_4 <= p_radius2)
   {
      //std::cout << "intersecting with square !" << "\n";
      return true;
   }

   return false;
}

glm::vec3 get_nearest_edge(Entity* point, Entity* square)
{
    auto box_collision_geometry = *((CollisionGeometryAlignedBox*) square->collision_geometry_ptr);

   //square vertices
   float square_x0 = square->position.x - box_collision_geometry.length_x;
   float square_z0 = square->position.z;
   float square_x1 = square->position.x;
   float square_z1 = square->position.z + box_collision_geometry.length_z;

   float player_x = point->position.x;
   float player_z = point->position.z;

   float d_1 = squared_minimum_distance(glm::vec2(square_x0, square_z0), glm::vec2(square_x1, square_z0), glm::vec2(player_x, player_z));
   float d_2 = squared_minimum_distance(glm::vec2(square_x1, square_z0), glm::vec2(square_x1, square_z1), glm::vec2(player_x, player_z));
   float d_3 = squared_minimum_distance(glm::vec2(square_x1, square_z1), glm::vec2(square_x0, square_z1), glm::vec2(player_x, player_z));
   float d_4 = squared_minimum_distance(glm::vec2(square_x0, square_z1), glm::vec2(square_x0, square_z0), glm::vec2(player_x, player_z));

   if(d_1 <= d_2 && d_1 <= d_3 && d_1 <= d_4)
   {
      return glm::vec3(square_x0, 0, square_z0) - glm::vec3(square_x1, 0, square_z0);
   }
   else if(d_2 <= d_1 && d_2 <= d_3 && d_2 <= d_4)
   {
      return glm::vec3(square_x1, 0, square_z0) - glm::vec3(square_x1, 0, square_z1);
   }
   else if(d_3 <= d_1 && d_3 <= d_2 && d_3 <= d_4)
   {
      return glm::vec3(square_x1, 0, square_z1) - glm::vec3(square_x0, 0, square_z1);
   }
    else if(d_4 <= d_1 && d_4 <= d_2 && d_4 <= d_3)
   {
      return glm::vec3(square_x0, 0, square_z1) - glm::vec3(square_x0, 0, square_z0);
   }

   assert(false);
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


