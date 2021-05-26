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
   vec2 normal_vec        = vec2(0,0);
   CollisionOutcomeEnum collision_outcome;
};

struct Collision {
   bool is_collided  = false;
   float overlap     = 0;
   vec2 normal_vec;
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
bool intersects_vertically_with_aabb(Entity* entity, Player* player);
Collision get_horizontal_overlap_with_player(Entity* entity, Player* player);
float get_vertical_overlap_player_vs_slope(Entity* slope, Player* player);
float get_slope_height_at_position(vec3 position, Entity* slope);
float get_slope_height_at_position(float position, Entity* slope, bool is_x, bool is_z);
auto project_entity_into_slope(Entity* entity, Entity* ramp);
float get_slope_height_at_position_along_inclination_axis(float position, Entity* slope);
bool intersects_vertically_with_slope(Entity* slope, Entity* player);
CollisionData sample_terrain_height_at_player(Entity* player, Entity* entity); 
// CollisionData check_for_floor_below_player(Player* player);
// CollisionData check_for_floor_below_player_when_slope(Player* player, bool only_check_player_tunnelling);
RaycastTest check_for_floor_below_player(Player* player);
bool player_feet_center_touches_slope(Player* player, Entity* slope);


//@mark done
auto project_entity_into_slope(Entity* entity, Entity* ramp)
{
   // projects the 3D bounding box into the axis-aligned ramp inclined line.
   // this way we can retrieve what is the height of the slope at the entity bounds.
   struct slope_heights{
      float min;
      float max;
   } heights;

   auto slope_c = ramp->collision_geometry.slope;
   auto [x0, x1, z0, z1] = entity->get_rect_bounds();
   float p0, p1;

   if(!is_float_zero(slope_c.tangent.x))
   {
      p0 = get_slope_height_at_position(x0, ramp, true, false);
      p1 = get_slope_height_at_position(x1, ramp, true, false);
   }
   else if(!is_float_zero(slope_c.tangent.z))
   {
      p0 = get_slope_height_at_position(z0, ramp, false, true);
      p1 = get_slope_height_at_position(z1, ramp, false, true);
   }
   else assert(false);

   heights.min = min(p0, p1);
   heights.max = max(p0, p1);

   return heights;
}

//@mark done
Collision get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player)
{
   // assumes we already checked that we have vertical intersection 
   auto aabb = entity->collision_geometry.aabb;
   auto p_collision = player->collision_geometry.cylinder;

   if(player->position.y >= entity->position.y)
   {
      float box_top       = entity->position.y + aabb.height;
      float player_bottom = player->position.y - p_collision.half_length;
      return Collision{true, box_top - player_bottom, vec2(0, 1)};
   }
   else
   {
      float box_bottom = entity->position.y;
      float player_top = player->position.y + p_collision.half_length;
      return Collision{true, player_top - box_bottom, vec2(0, -1)};
   }
}

//@mark done
bool intersects_vertically_with_aabb(Entity* entity, Player* player)
{
   // if we are standing in a slope, then we need to check which player height we want to check against,
   // because at each point that the player touches the slope he has a different height
   if(player->player_state == PLAYER_STATE_STANDING &&
      player->standing_entity_ptr->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
   {
      float box_height  = entity->collision_geometry.aabb.height;
      float box_top     = entity->position.y + box_height;
      float box_bottom  = entity->position.y;
      float p_feet = player->entity_ptr->position.y - player->half_height;
      float player_top  = player->entity_ptr->position.y + player->half_height;
      if(p_feet > box_top || player_top < box_bottom)
         return false;

      auto ramp = player->standing_entity_ptr;

      auto [p_min_y, p_max_y]     = project_entity_into_slope(player->entity_ptr, ramp);
      auto [box_min_y, box_max_y] = project_entity_into_slope(entity, ramp);

      box_top    = min(box_max_y, box_top);
      box_bottom = max(box_min_y, box_bottom);

      // computes what should be the player's bottom position considering the
      // box position in slope (before or after player)
      float p_bottom_for_collision;
      if(box_max_y <= p_feet)
         p_bottom_for_collision = p_feet;
      else if (box_min_y >= p_max_y)
         p_bottom_for_collision = p_max_y;
      else 
      {
         if(entity->name == "jose")
         {
            cout << "box_min_y: " << box_min_y << ", box_max_y: " << box_max_y << "\n";
            cout << "p_feet: " << p_feet << ", p_max_y: " << p_max_y << "\n";
         }  
      }

      // checks if entity overlaps player vertically
      return p_bottom_for_collision + COLLISION_EPSILON < box_top && player_top > box_bottom + COLLISION_EPSILON;
   }
   else
   {
      float player_bottom = player->entity_ptr->position.y - player->half_height;
      float player_top = player->entity_ptr->position.y + player->half_height;

      float box_top = entity->position.y + entity->collision_geometry.aabb.height;
      float box_bottom = entity->position.y;
      
      return player_bottom + COLLISION_EPSILON < box_top && player_top > box_bottom + COLLISION_EPSILON;
   }
}

//mark done
Collision get_horizontal_overlap_with_player(Entity* entity, Player* player)
{
   //now this serves for slopes and aabbs too.
   auto [x0, x1, z0, z1] = entity->get_rect_bounds();

   float player_x = player->entity_ptr->position.x;
   float player_z = player->entity_ptr->position.z; 

   if (x0 <= player_x && x1 >= player_x && z0 <= player_z && z1 >= player_z) 
   {
      // overlap = 0
      return Collision{true};    
   }
   else
   {
      // n_vec = vector from player to nearest point in rectangle surface
      float nx = std::max(x0, std::min(x1, player_x));
      float nz = std::max(z0, std::min(z1, player_z));
      vec2 n_vec = vec2(nx, nz) - vec2(player_x, player_z);
      float distance = glm::length(n_vec);
      float overlap = player->radius - distance;

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


float get_vertical_overlap_player_vs_slope(Entity* slope, Player* player)
{
   // assumes we already checked that we have vertical intersection 
   float y = get_slope_height_at_position(player->entity_ptr->position, slope);
   float player_bottom = player->entity_ptr->position.y - player->half_height;
   return y - player_bottom;
}


float get_slope_height_at_position(vec3 position, Entity* slope)
{
   // this will check the slope orientation for us and do the right thing
   float height = -1;
   auto slope_c = slope->collision_geometry.slope;
   float slope_top = slope->position.y + slope_c.height;

   if (!is_float_zero(slope_c.tangent.x))
      height = slope_top - slope_c.inclination * slope_c.tangent.x * (position.x - slope->position.x);
   else if (!is_float_zero(slope_c.tangent.z))
      height = slope_top - slope_c.inclination * slope_c.tangent.z * (position.z - slope->position.z);
   else assert(false);

   return height; 
}

float get_slope_height_at_position(float position, Entity* slope, bool is_x, bool is_z)
{
   // this will assume we checked the slope orientation before calling it
   float height = -1;
   auto slope_c = slope->collision_geometry.slope;
   float slope_top = slope->position.y + slope_c.height;

   if (is_x)
      height = slope_top - slope_c.inclination * slope_c.tangent.x * (position - slope->position.x);
   else if (is_z)
      height = slope_top - slope_c.inclination * slope_c.tangent.z * (position - slope->position.z);
   else assert(false);

   return height;
}

bool intersects_vertically_with_slope(Entity* slope, Entity* player)
{
   float slope_top      = slope->position.y + slope->collision_geometry.slope.height;
   float slope_bottom   = slope->position.y;

   auto p_col           = player->collision_geometry.cylinder;
   float player_bottom  = player->position.y - p_col.half_length;
   float player_top     = player->position.y + p_col.half_length;

   // basic cull in y 
   if(player_bottom  >= slope_top || player_top <= slope_bottom)
      return false;

   // gets the player projection into the slope
   auto [p_min_y, p_max_y] = project_entity_into_slope(player, slope);
   float diff_top          = slope_top - p_min_y;
   float diff_bottom       = p_max_y - slope_bottom;

   // player is not touching the ramp
   if(p_max_y < player_bottom)
      return false;

   // second line will be useful when player is 'outside' slope and touches it front-on
   return
      (diff_top > 0     && diff_bottom > 0   ) &&
      (diff_top > 0.08  || diff_bottom > 0.08) && 
      (player_bottom < p_max_y);
}

//@mark done
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
   auto [x0, x1, z0, z1] = entity->get_rect_bounds();
   float height;

   switch(entity->collision_geometry_type)
   {
      case COLLISION_ALIGNED_BOX:
         height = entity->position.y + entity->collision_geometry.aabb.height;
         break;
      case COLLISION_ALIGNED_SLOPE:
         height = get_slope_height_at_position(player->position, entity);
         break;
      default:
         return cd;
   } 

   float player_x = player->position.x;
   float player_z = player->position.z;

   cd.overlap = height;

   // check if player's center lies inside terrain box
   if(x0 <= player_x && x1 >= player_x && z0 <= player_z && z1 >= player_z)
      cd.is_collided = true;

   return cd;
}

RaycastTest check_for_floor_below_player(Player* player)
{
   // cast ray slightly above contact point to catch tunneling correctly
   float tolerance = 0.01;
   auto downward_ray = Ray{player->feet() + vec3{0.0f, tolerance, 0.0f}, vec3{0.0f, -1.0f, 0.0f}};
   RaycastTest raytest = test_ray_against_scene(downward_ray);
   if(raytest.hit && raytest.distance < tolerance)
   {
      cout << "distance from ray to floor: " << raytest.distance << "\n";
      return raytest;
   }
   else
      return RaycastTest{false};
}

bool player_feet_center_touches_slope(Player* player, Entity* slope)
{
// cast ray slightly above contact point to catch tunneling correctly
   float tolerance = 0.01;
   auto downward_ray = Ray{player->feet() + vec3{0.0f, tolerance, 0.0f}, vec3{0.0f, -1.0f, 0.0f}};
   RaycastTest raytest = test_ray_against_entity(downward_ray, slope);
   if(raytest.hit && raytest.distance < tolerance)
   {
      return true;
   }
   else return false;
}