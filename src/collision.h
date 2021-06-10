#include <algorithm>
#include <math.h>

const float COLLISION_EPSILON = 0.0001f;

enum CollisionType{
   HORIZONTAL = 0,
   VERTICAL = 1
};

enum CollisionOutcomeEnum {
   JUMP_SUCCESS                = 0,  // player fell succesfully into another platform
   // JUMP_FAIL                   = 1,  // player fell into the edge of another platform
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
   vec2 normal_vec   = vec2(0.0f);
   bool is_inside    = false;
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
float get_distance_from_slope(Entity* slope, Player* player);
float get_slope_height_at_position(vec3 position, Entity* slope);
float get_slope_height_at_position(float position, Entity* slope, bool is_x, bool is_z);
auto project_entity_into_slope(Entity* entity, Entity* ramp);
float get_slope_height_at_position_along_inclination_axis(float position, Entity* slope);
bool intersects_vertically_with_slope(Entity* slope, Entity* player);
CollisionData get_terrain_height_at_player(Entity* player, Entity* entity);
RaycastTest check_for_floor_below_player(Player* player);
bool player_feet_center_touches_slope(Player* player, Entity* slope);
CollisionData check_collision_horizontal(
      Player* player, EntityBufferElement* entity_iterator, int entity_list_size
); 
CollisionData check_collision_vertical(Player* player, EntityBufferElement* entity_iterator, int entity_list_size);
bool check_event_trigger_collision(Entity* trigger, Entity* player);


float SLIDE_MAX_ANGLE = 1.4;
float SLIDE_MIN_ANGLE = 0.6;

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


bool intersects_vertically_with_aabb(Entity* entity, Player* player)
{
   // if we are standing in a slope, then we need to check which player height we want to check against,
   // because at each point that the player touches the slope he has a different height
   // @todo: shouldnt here we ask (player_qualifies_as_standing) ? If sliding, doesnt this matter too?
   if(player->player_state == PLAYER_STATE_STANDING &&
      player->standing_entity_ptr->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
   {
      float box_height  = entity->collision_geometry.aabb.height;
      float box_top     = entity->position.y + box_height;
      float box_bottom  = entity->position.y;
      float p_feet      = player->entity_ptr->position.y - player->half_height;
      float player_top  = player->entity_ptr->position.y + player->half_height;

      // basic y cull
      if(p_feet + COLLISION_EPSILON >= box_top || player_top - COLLISION_EPSILON <= box_bottom)
         return false;

      auto ramp = player->standing_entity_ptr;

      auto [p_min_y, p_max_y]     = project_entity_into_slope(player->entity_ptr, ramp);
      auto [box_min_y, box_max_y] = project_entity_into_slope(entity, ramp);

      // box_top    = min(box_max_y, box_top);
      // box_bottom = max(box_min_y, box_bottom);

      // computes what should be the player's bottom position considering the
      // box position in slope (before or after player)
      float p_bottom_for_collision;
      if (box_max_y <= p_feet)         p_bottom_for_collision = p_feet;
      else                             p_bottom_for_collision = p_max_y;

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


Collision get_horizontal_overlap_with_player(Entity* entity, Player* player)
{
   // If the player's center lies inside entity, returns that it is.
   // If not, but there is overlap, returns the actual overlap.

   //now this serves for slopes and aabbs too.
   auto [x0, x1, z0, z1] = entity->get_rect_bounds();

   float player_x = player->entity_ptr->position.x;
   float player_z = player->entity_ptr->position.z;  

   // player is inside rect bounds
   if (x0 <= player_x && x1 >= player_x && z0 <= player_z && z1 >= player_z) 
   {
     Collision check;
     check.is_inside = true;
     check.is_collided = true;
     return check;  
   }  

   // n_vec = surface-normal vector from player to nearest point in rectangle surface
   float nx = std::max(x0, std::min(x1, player_x));
   float nz = std::max(z0, std::min(z1, player_z));
   vec2 n_vec = vec2(nx, nz) - vec2(player_x, player_z);
   float distance = glm::length(n_vec);
   float overlap = player->radius - distance;

   return overlap > COLLISION_EPSILON ?
      Collision{true, overlap, glm::normalize(n_vec), false} :
      Collision{false};
}


float get_distance_from_slope(Entity* slope, Player* player)
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

   // basic cull in y (we use epsilon so if same height no collision is detected)
   if(player_bottom + COLLISION_EPSILON  >= slope_top || player_top - COLLISION_EPSILON <= slope_bottom)
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


// dont know what to do with this yet
float get_vertical_overlap_player_slope(Entity* slope, Entity* player)
{
   // 
   float slope_top      = slope->position.y + slope->collision_geometry.slope.height;

   auto p_col           = player->collision_geometry.cylinder;
   float player_bottom  = player->position.y - p_col.half_length;
   float player_top     = player->position.y + p_col.half_length;

   // gets the player projection into the slope
   auto [p_min_y, p_max_y] = project_entity_into_slope(player, slope);
   float diff_top          = slope_top - p_min_y;

   // player is not touching the ramp
   if(p_max_y < player_bottom) return -1;

   if(p_max_y > slope_top) return slope_top - player_bottom;
   else                    return p_max_y - player_bottom;
}


CollisionData get_terrain_height_at_player(Entity* player, Entity* entity)
{
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
   // cast ray slightly above contact point to catch slope tunneling correctly
   // (we only get to this point with low inclination slopes, the other ones trigger collision before this point)
   float tunneling_tolerance = 0.03;
   auto downward_ray = Ray{player->feet() + vec3{0.0f, tunneling_tolerance, 0.0f}, vec3{0.0f, -1.0f, 0.0f}};
   RaycastTest raytest = test_ray_against_scene(downward_ray);

   if(!raytest.hit) return RaycastTest{false};


   // because slopes are inclined, if we are standing and moving towards a slope and we
   // dont allow more tolerance in detection, we will trigger a player fall
   float detection_tolerance = tunneling_tolerance + 0.01;
   // if(raytest.entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
   //     detection_tolerance = 0.0;

   if(raytest.distance <= detection_tolerance)
      return RaycastTest{true, raytest.distance - tunneling_tolerance, raytest.entity};
   else
      return RaycastTest{false};
}

CollisionData check_collision_horizontal(Player* player, EntityBufferElement* entity_iterator, int entity_list_size) 
{
   CollisionData return_cd; 
   // this serves only to enable us to check for standing_entity_ptr, otherwise its NULL and we get an exception
   bool player_qualifies_as_standing = 
      player->player_state == PLAYER_STATE_STANDING || 
      player->player_state == PLAYER_STATE_SLIDING  ||
      player->player_state == PLAYER_STATE_SLIDE_FALLING;

   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;
	   float biggest_overlap = -1;
      Collision collision;
      bool set_collided_entity = false;
      bool entity_is_not_player_current_ground = !(player_qualifies_as_standing && player->standing_entity_ptr == entity);
	   if (entity_iterator->collision_check == false && entity_is_not_player_current_ground)
      {    
         // AABB
         if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX && intersects_vertically_with_aabb(entity, player))
         {
            collision = get_horizontal_overlap_with_player(entity, player);

            if(collision.is_collided && collision.overlap > biggest_overlap)
            {
               return_cd.collision_outcome = BLOCKED_BY_WALL;
               set_collided_entity = true;
            }
         }

         // ALIGNED SLOPE
         else if (entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
         {
            // get some info about slope
            auto col_geometry = entity->collision_geometry.slope;
            auto slope_2d_tangent = glm::normalize(vec2(col_geometry.tangent.x, col_geometry.tangent.z));

            collision = get_horizontal_overlap_with_player(entity, player);

            // return early if not collided or player is inside slope
            if(!collision.is_collided || collision.overlap < biggest_overlap)
            {
               entity_iterator++;
               continue;
            }

            // player is facing slope inclined face
            if(is_vec2_equal(collision.normal_vec, -1.0f * slope_2d_tangent))
            {
               // if slope is very inclined...
               if(player_qualifies_as_standing && col_geometry.inclination > SLIDE_MIN_ANGLE)
               {
                  // ...then, we care if the player cylinder touches the slope in any way (cylinder tips count)
                  if(intersects_vertically_with_slope(entity, player->entity_ptr))
                  {
                     set_collided_entity = true;
                     return_cd.collision_outcome = BLOCKED_BY_WALL;
                  }
               }
               else
               {
                  // either player is (falling) OR (standing but slope is not angled enough)...
                  // we should only care for collisions between slope and the player center in x-z (feet center)
                  float v_overlap = get_distance_from_slope(entity, player);
                  if(v_overlap > 0)
                  {
                     set_collided_entity = true;
                     return_cd.collision_outcome = STEPPED_SLOPE;
                     collision.overlap = v_overlap; // @WORKAROUND
                  }
               }

            }
            // player is not facing slope inclined face
            else
            {
               // ...then we care if player (as a cylinder) is touching the slope
               if(intersects_vertically_with_slope(entity, player->entity_ptr))
               {
                  set_collided_entity = true;
                  return_cd.collision_outcome = BLOCKED_BY_WALL;
               }
            }
         }

         // set current entity as collided one
         if(set_collided_entity)
         {
            // cout << "horizontal collision with '" << entity->name << "'\n";
            biggest_overlap = collision.overlap;

            return_cd.is_collided = true;
            return_cd.collided_entity_ptr = entity;
            return_cd.overlap = collision.overlap;
            return_cd.normal_vec = collision.normal_vec;
         }
      }
      entity_iterator++;
   }
   return return_cd;
}


CollisionData check_collision_vertical(Player* player, EntityBufferElement* entity_iterator, int entity_list_size)
{
   CollisionData return_cd; 
   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;
	   float biggest_overlap = -1;
	   if (entity_iterator->collision_check == false)
      {   
         float vertical_overlap = -1;
         Collision v_overlap_collision; // for ceilling hit detection
         Collision horizontal_check;
         
         // A) CHECKS ENTITY GEOMETRIC TYPE
         if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
         {
            if(!intersects_vertically_with_aabb(entity, player))
            { 
               entity_iterator++; 
               continue; 
            }
            v_overlap_collision = get_vertical_overlap_player_vs_aabb(entity, player->entity_ptr);
            vertical_overlap = v_overlap_collision.overlap;
            horizontal_check = get_horizontal_overlap_with_player(entity, player);
         }
         else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
         {
            auto top = entity->position.y + entity->get_height();
            if(top < player->feet().y)
            { 
               entity_iterator++; 
               continue; 
            }
            // here we should get tunneling
            vertical_overlap = get_distance_from_slope(entity, player);
            horizontal_check = get_horizontal_overlap_with_player(entity, player);   
         }
         

         // B) CHECKS IF ANYTHING WORHTWHILE HAPPENED
         if(horizontal_check.is_collided && vertical_overlap >= 0 && vertical_overlap > biggest_overlap)
         {
            biggest_overlap = vertical_overlap;
            return_cd.collided_entity_ptr = entity;

            // player fell inside a slope
            if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE &&
               horizontal_check.is_inside)
            {
               auto col_geometry = entity->collision_geometry.slope;
               if(col_geometry.inclination > SLIDE_MAX_ANGLE)
               {
                  return_cd.overlap = vertical_overlap;
                  return_cd.collision_outcome = JUMP_SLIDE_HIGH_INCLINATION;
               }
               else if(col_geometry.inclination > SLIDE_MIN_ANGLE)
               {
                  return_cd.overlap = vertical_overlap;
                  return_cd.collision_outcome = JUMP_SLIDE;
               }
               else
               {
                  return_cd.overlap = vertical_overlap;
                  return_cd.collision_outcome = JUMP_SUCCESS;
               }
            }

            // player jumped and hit the ceiling
            // @TODO: maybe set vertical_overlap > 0 here?
            else if(
               v_overlap_collision.normal_vec.y == -1 &&
               player->prior_position.y < player->entity_ptr->position.y &&
               vertical_overlap < 0.03)
            {
               return_cd.overlap = vertical_overlap;
               return_cd.normal_vec = v_overlap_collision.normal_vec;
               return_cd.collision_outcome = JUMP_CEILING;
            }

            // player has part of body outside the entity bounds
            // and is overlapping too much (from top of entity to player feet)
            else if(!horizontal_check.is_inside && vertical_overlap > 0.05) 
            {
               return_cd.overlap = horizontal_check.overlap;
               return_cd.normal_vec = horizontal_check.normal_vec;
               return_cd.collision_outcome = JUMP_FACE_FLAT;
            }

            // player did not get half body inside platform (didnt make the jump)
            // else if(horizontal_check.overlap < player->radius)
            // {
            //    return_cd.overlap = vertical_overlap;
            //    return_cd.normal_vec = horizontal_check.normal_vec;
            //    return_cd.collision_outcome = JUMP_FAIL;
            // }

            // got at least half body inside platform (made the jump)
            else
            {
               return_cd.overlap = vertical_overlap;
               return_cd.collision_outcome = JUMP_SUCCESS;
            }
         }
      }
      entity_iterator++;
   }
   return return_cd;
}

bool check_event_trigger_collision(Entity* trigger, Entity* player)
{
   auto col2 = player->collision_geometry.cylinder;
   auto top1 = trigger->position.y + trigger->trigger_scale.y * 2;
   auto top2 = player->position.y + col2.half_length * 2;
   auto bottom1 = trigger->position.y;
   auto bottom2 = player->position.y;

   if(top1 < bottom2 || bottom1 > top2)
      return false;
   
   auto trigger_pos_2d = vec2{trigger->trigger_pos.x, trigger->trigger_pos.z};
   auto player_pos_2d = vec2{player->position.x, player->position.z};
   auto dist_2d = glm::length(trigger_pos_2d - player_pos_2d);
   auto radius_sum = trigger->trigger_scale.x + col2.radius;

   if(radius_sum < dist_2d)
      return false;

   return true;
}