#include <algorithm>
#include <math.h>

const float COLLISION_EPSILON = 0.0001f;

enum CollisionType{
   HORIZONTAL = 0,
   VERTICAL = 1
};

enum CollisionOutcomeEnum {
   JUMP_SUCCESS                = 0,  // player fell succesfully into another platform
   JUMP_FAIL                   = 1,  // player fell into the edge of another platform
   JUMP_FACE_FLAT              = 2,  // player fell with face flat on wall
   JUMP_SLIDE                  = 3,  // player is sliding
   JUMP_SLIDE_HIGH_INCLINATION = 4,  // player is sliding but cant move
   JUMP_CEILING                = 5,  // player hit the ceiling with his head
   STEPPED_SLOPE               = 6,  // player entered a low inclination slope
   BLOCKED_BY_WALL             = 7   // player hit a wall
};

struct CollisionData {
   bool is_collided            = false;
   Entity* collided_entity_ptr = NULL;
   float overlap               = 0;
   glm::vec2 normal_vec        = glm::vec2(0,0);
   CollisionOutcomeEnum collision_outcome;
};

struct Collision {
   bool is_collided  = false;
   float overlap     = 0;
   glm::vec2 normal_vec;
};

struct Boundaries {
   float x0;
   float x1;
   float z0;
   float z1;
};

struct SlopeHeightsPlayer
{
   float at_coord_0;
   float at_coord_c;
   float at_coord_1;
   float min;
   float max;
};



Collision get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player);
float get_vertical_overlap_player_vs_slope(Entity* entity, Entity* player);

CollisionData sample_terrain_height_at_player(Entity* player, Entity* entity); 
bool check_2D_collision_circle_and_aligned_square(Entity* circle, Entity* square);
float squared_distance_between_point_and_line(float x1, float y1, float x2, float y2, float x0, float y0);
float squared_minimum_distance(glm::vec2 v, glm::vec2 w, glm::vec2 p);
glm::vec3 get_nearest_edge(Entity* point, Entity* square);
bool intersects_vertically(Entity* entity, Player* player);
bool intersects_vertically_slope(Entity* entity, Entity* player);
bool intersects_vertically_standing_slope(Entity* entity, Player* player);
Collision get_horizontal_overlap_player_aabb(Entity* entity, Entity* player);
Collision get_horizontal_overlap_player_slope(Entity* entity, Entity* player);
float get_slope_height_at_player_position(Entity* player, Entity* entity);
Boundaries get_slope_boundaries(Entity* entity);
SlopeHeightsPlayer get_slope_heights_at_player(Entity* player, Entity* entity);
CollisionData check_for_floor_below_player(Player* player);
CollisionData check_for_floor_below_player_when_slope(Player* player, bool only_check_player_tunnelling);



//_____________________________________
// 
// ENTITY COLLISION DETECTION FUNCTIONS
//_____________________________________

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

   if (box_x0 <= player_x && box_x1 >= player_x && box_z0 <= player_z && box_z1 >= player_z)
   {
      return Collision{true}; // overlap = 0
   }
   else
   {
      float nx = std::max(box_x0, std::min(box_x1, player_x));
      float nz = std::max(box_z0, std::min(box_z1, player_z));
      // vector from player to nearest point in rectangle surface
      glm::vec2 n_vec = glm::vec2(nx, nz) - glm::vec2(player_x, player_z);
      float distance = glm::length(n_vec);
      float p_radius = player_collision_geometry->radius;
      float overlap = p_radius - distance;

      Collision c;
      if(overlap > COLLISION_EPSILON)
      {   
         c.is_collided = true;
         c.overlap = overlap;
         c.normal_vec = glm::normalize(n_vec);
      }

      return c;
   }     
}

Collision get_horizontal_overlap_player_slope(Entity* entity, Entity* player)
{
   //box boundaries
   auto bounds = get_slope_boundaries(entity);

   float player_x = player->position.x;
   float player_z = player->position.z;

   float nx = std::max(bounds.x0, std::min(bounds.x1, player_x));
   float nz = std::max(bounds.z0, std::min(bounds.z1, player_z));

   if (bounds.x0 <= player_x && bounds.x1 >= player_x && bounds.z0 <= player_z && bounds.z1 >= player_z)
   {
      return Collision{true}; // overlap = 0
   }
   else
   {
      // vector from player to nearest point in rectangle surface
      glm::vec2 n_vec = glm::vec2(nx, nz) - glm::vec2(player_x, player_z);
      float distance = glm::length(n_vec);
      auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
      float p_radius = player_collision_geometry->radius;
      float overlap = p_radius - distance;

      Collision c;
      if(overlap > COLLISION_EPSILON)
      {   
         c.is_collided = true;
         c.overlap = overlap;
         c.normal_vec = glm::normalize(n_vec);
      }

      return c;
   }     
}


Collision get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player)
{
   // assumes we already checked that we have vertical intersection 
   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;

   if(player->position.y >= entity->position.y)
   {
      float box_top = entity->position.y + box_collision_geometry.length_y;
      float player_bottom = player->position.y - player_collision_geometry->half_length;
      return Collision{true, box_top - player_bottom, glm::vec2(0, 1)};
   }
   else
   {
      float box_bottom = entity->position.y;
      float player_top = player->position.y + player_collision_geometry->half_length;
      return Collision{true, player_top - box_bottom, glm::vec2(0, -1)};
   }
}

float get_vertical_overlap_player_vs_slope(Entity* entity, Entity* player)
{
   // assumes we already checked that we have vertical intersection 
   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   float player_bottom = player->position.y - player_collision_geometry->half_length;

   float y = get_slope_height_at_player_position(player, entity);
   return y - player_bottom;
}


// _____________________________________
//
// CULLING / UTILITY COLLISION FUNCTIONS
// _____________________________________

bool intersects_vertically(Entity* entity, Player* player)
{
   // if we are standing in a slope, then we need to check which player height we want to check against,
   // because at each point that the player touches the slope he has a different height
   if(player->player_state == PLAYER_STATE_STANDING &&
      player->standing_entity_ptr->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
   {
      return intersects_vertically_standing_slope(entity, player);
   }
   else
   {
      auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->entity_ptr->collision_geometry_ptr;
      float player_bottom = player->entity_ptr->position.y - player_collision_geometry->half_length;
      float player_top = player->entity_ptr->position.y + player_collision_geometry->half_length;

      auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
      float box_top = entity->position.y + box_collision_geometry.length_y;
      float box_bottom = entity->position.y;
      
      return player_bottom + COLLISION_EPSILON < box_top && player_top > box_bottom + COLLISION_EPSILON;
   }
}

bool intersects_vertically_standing_slope(Entity* entity, Player* player)
{
   // this function is for boxes only
   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->entity_ptr->collision_geometry_ptr;
   float player_bottom = player->entity_ptr->position.y - player_collision_geometry->half_length;
   float player_top = player->entity_ptr->position.y + player_collision_geometry->half_length;

   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
   float box_top = entity->position.y + box_collision_geometry.length_y;
   float box_bottom = entity->position.y;

   float box_x0 = entity->position.x;
   float box_x1 = entity->position.x + box_collision_geometry.length_x;
   float box_z0 = entity->position.z;
   float box_z1 = entity->position.z + box_collision_geometry.length_z;
      

   auto ramp = player->standing_entity_ptr;
   auto y_values = get_slope_heights_at_player(player->entity_ptr, ramp);

  
   float player_bottom_pos;

   // to facillitate understanding this, imagine the slope
   // now imagine the cylinder in the slope, what is the player bottom to be considered for
   // collision? the player bottom or the point of contact with the slope (always higher) ?
   // (to check the ifs accurately, draw x axis and slope at 0 and 180 degrees with player on it and objects behind and after it)
   
   
   if(ramp->rotation.y == 0)
   {  // along x axis
      if(entity->position.x >= box_x0)
      {
         player_bottom_pos = y_values.max;
      }
      else
      {
         player_bottom_pos = player_bottom;
      }
   }
   else if(ramp->rotation.y == 90)
   {  // along negative z
      if(entity->position.z <= box_z1)
      {
         player_bottom_pos = y_values.max;
      }
      else
      {
         player_bottom_pos = player_bottom;
      }
      
   }
   else if(ramp->rotation.y == 180)
   {  // along negative x
      if(entity->position.x <= box_x1)
      {
         player_bottom_pos = y_values.max;
      }
      else
      {
         player_bottom_pos = player_bottom;
      }
   }
   else if(ramp->rotation.y == 270)
   {  // along positive z
      if(entity->position.z >= box_z0)
      {
         player_bottom_pos = y_values.max;
      }
      else
      {
         player_bottom_pos = player_bottom;
      }
   }
   else
   {
      assert(false);
   }

   return player_bottom_pos + COLLISION_EPSILON < box_top && player_top > box_bottom + COLLISION_EPSILON;
}

bool intersects_vertically_slope(Entity* entity, Entity* player)
{
   // since a slope has a diagonal profile in its cross section, we need to sample the points
   // that the player 
   auto col_geometry = *((CollisionGeometrySlope*) entity->collision_geometry_ptr);
   float slope_top = entity->position.y + col_geometry.slope_height;
   float slope_bottom = entity->position.y;

   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   float player_bottom = player->position.y - player_collision_geometry->half_length;
   float player_top = player->position.y + player_collision_geometry->half_length;

   auto y_values = get_slope_heights_at_player(player, entity);

   // first we clip horizontally, checking if the y_values calculated based on player horizontal position are
   // inside the [slope_bottom, slope_top] range
   // then we check that the player is intersecting vertically with the slope using his current y positions
   if(player_bottom >= slope_top || player_top <= slope_bottom)
      return false;

   if(y_values.min < slope_top && y_values.max > slope_bottom && player_bottom < y_values.max && player_top > slope_bottom)
      return true;

   return false;
}


SlopeHeightsPlayer get_slope_heights_at_player(Entity* player, Entity* entity)
{
   auto player_position = player->position;
   float at_coord_0;
   float at_coord_c;
   float at_coord_1;

   auto pcg = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   at_coord_c = at_coord_0 = get_slope_height_at_player_position(player, entity);

   auto slope_rot = (int) entity->rotation.y;
   if(slope_rot == 0 || slope_rot == 180)
   {
      player->position.x -= pcg->radius;
      at_coord_0 = get_slope_height_at_player_position(player, entity);
      player->position.x += pcg->radius * 2;
      at_coord_1 = get_slope_height_at_player_position(player, entity);
   }
   else if(slope_rot == 90 || slope_rot == 270)
   {
      player->position.z -= pcg->radius;
      at_coord_0 = get_slope_height_at_player_position(player, entity);
      player->position.z += pcg->radius * 2;
      at_coord_1 = get_slope_height_at_player_position(player, entity);
   }
   else
   {
      assert(false);        
   }

   player->position = player_position;

   float min_y = min(at_coord_0, at_coord_1);
   float max_y = max(at_coord_0, at_coord_1);

   return SlopeHeightsPlayer{at_coord_0, at_coord_c, at_coord_1, min_y, max_y};
}

float get_slope_height_at_player_position(Entity* player, Entity* entity)
{
   auto col_geometry = *((CollisionGeometrySlope*) entity->collision_geometry_ptr);
   float slope_top = entity->position.y + col_geometry.slope_height;
   float a = col_geometry.slope_height / col_geometry.slope_length;

   float y;
   switch((int) entity->rotation.y)
   {
      case 0:  // positive x
      {
         float x = player->position.x - entity->position.x;
         y = slope_top - a * x;
         break;
      }
      case 90: // negative z
      {
         float z = player->position.z - entity->position.z;
         y = slope_top + a * z;
         break;
      }
      case 180: // negative x
      {
         float x = player->position.x - entity->position.x;
         y = slope_top + a * x;
         break;
      }
      case 270: // positive z
      {
         float z = player->position.z - entity->position.z;
         y = slope_top - a * z;
         break;
      }
   }

   return y;
}

Boundaries get_slope_boundaries(Entity* entity)
{
   auto box_collision_geometry = *((CollisionGeometrySlope*) entity->collision_geometry_ptr);

   float x1;
   float z1;
   float x0;
   float z0;   
   switch((int) entity->rotation.y)
   {
      case 0:
      {
         x0 = entity->position.x;
         z0 = entity->position.z;
         x1 = entity->position.x + box_collision_geometry.slope_length;
         z1 = entity->position.z + box_collision_geometry.slope_width;
         break;
      }
      case 90:
      {
         x0 = entity->position.x;
         z0 = entity->position.z - box_collision_geometry.slope_length;
         x1 = entity->position.x + box_collision_geometry.slope_width;
         z1 = entity->position.z;
         break;
      }
      case 180:
      {
         x0 = entity->position.x - box_collision_geometry.slope_length;
         z0 = entity->position.z - box_collision_geometry.slope_width;
         x1 = entity->position.x;
         z1 = entity->position.z;
         break;
      }
      case 270:
      {
         x0 = entity->position.x - box_collision_geometry.slope_width;
         x1 = entity->position.x;
         z1 = entity->position.z + box_collision_geometry.slope_length;
         z0 = entity->position.z;
         break;
      }
   }

   return Boundaries { x0, x1, z0, z1 };
}


CollisionData sample_terrain_height_at_player(Entity* player, Entity* entity)
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

   CollisionData cd;
   float height;
   float x0;
   float x1;
   float z0;
   float z1;
   if (entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
   {
      auto collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);

      //platform boundaries
      x0 = entity->position.x;
      x1 = entity->position.x + collision_geometry.length_x;
      z0 = entity->position.z;
      z1 = entity->position.z + collision_geometry.length_z;

      float box_top = entity->position.y + collision_geometry.length_y;
      height = box_top;
   }
   else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
   {
      auto collision_geometry = *((CollisionGeometrySlope*) entity->collision_geometry_ptr);
      auto bounds = get_slope_boundaries(entity);
      x0 = bounds.x0;
      x1 = bounds.x1;
      z0 = bounds.z0;
      z1 = bounds.z1;
      
      height = get_slope_height_at_player_position(player, entity);;
   }
   else
   {
      return cd;
      // assert(false);
   }

   float player_x = player->position.x;
   float player_z = player->position.z;

   float min_x = min(x0, x1);
   float max_x = max(x0, x1);
   float min_z = min(z0, z1);
   float max_z = max(z0, z1);

   cd.overlap = height;

   // check if player's center lies inside terrain box
   if(min_x <= player_x && max_x >= player_x && min_z <= player_z && max_z >= player_z)
   {
      cd.is_collided = true;
   }

   return cd;
}

CollisionData check_for_floor_below_player(Player* player)
{
   Entity **entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
   float min_distance = 0.08;  // CONTROLS MAX HEIGHT FOR PLAYER TO NOT FALL STRAIGHT WHEN QUITTING PLATFORM
   CollisionData response;
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator;

      auto check = sample_terrain_height_at_player(player->entity_ptr, entity);
      float y_diff = (player->entity_ptr->position.y - player->half_height) - check.overlap; //here overlap is height...

      // if player is standing this will check for any platform just below him
      // if player is sliding from super inclined slope this will check if player punched through
      if(check.is_collided && y_diff >= 0 && y_diff < min_distance) 
      {
         min_distance = y_diff;
         response.is_collided = true;
         response.overlap = y_diff;
         response.collided_entity_ptr = entity;
      }
      entity_iterator++;
   }
   return response;
}

CollisionData check_for_floor_below_player_when_slope(Player* player, bool only_check_player_tunnelling = false)
{
   Entity **entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
   float min_distance = only_check_player_tunnelling ? 0 : 0.08;
   CollisionData response;
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator;
      if(entity != player->standing_entity_ptr)
      {
         auto check = sample_terrain_height_at_player(player->entity_ptr, entity);
         float y_diff = (player->entity_ptr->position.y - player->half_height) - check.overlap; //here overlap is height...
         float y_diff_check = only_check_player_tunnelling ? y_diff : abs(y_diff);
         if(check.is_collided && y_diff_check < min_distance) 
         {
            min_distance = y_diff;
            response.is_collided = true;
            response.overlap = y_diff;
            response.collided_entity_ptr = entity;
            int sign = y_diff < 0 ? -1 : 1;   // say if player is above or below detected floor
            response.normal_vec = glm::vec2(0, sign);
         }
      }
      entity_iterator++;
   }
   return response;
}


//_________
//
// LEGACY ?
//_________

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

   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) circle->collision_geometry_ptr;
   float p_radius2 = player_collision_geometry->radius * player_collision_geometry->radius;
   std::cout << "radius2: " << p_radius2 << "\n";
   if(d_1 <= p_radius2 || d_2 <= p_radius2 || d_3 <= p_radius2 || d_4 <= p_radius2)
   {
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
   return glm::vec3();
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


