enum CollisionOutcomeEnum {
   JUMP_SUCCESS                              = 0,  // player fell succesfully into another platform
// JUMP_FAIL                                 = 1,  // player fell into the edge of another platform
   JUMP_FACE_FLAT                            = 2,  // player fell with face flat on wall
   JUMP_SLIDE                                = 3,  // player is sliding
   JUMP_SLIDE_HIGH_INCLINATION               = 4,  // player is sliding but cant move
   JUMP_CEILING                              = 5,  // player hit the ceiling with his head
   STEPPED_SLOPE                             = 6,  // player entered a low inclination slope
   BLOCKED_BY_WALL                           = 7,  // player hit a wall
   NO_OUTCOME                                = 999
};

struct EntitiesCollision {
   bool is_collided                          = false;
   Entity* collided_entity_ptr               = NULL;
   float overlap                             = 0;
   vec2 normal_vec                           = vec2(0,0);
   CollisionOutcomeEnum collision_outcome;
};

#include <algorithm>
#include <math.h>
//#include <cl_log.h>


struct SlopeHeightsPlayer
{
   float at_coord_0;
   float at_coord_c;
   float at_coord_1;
   float min;
   float max;
};

float SLIDE_MAX_ANGLE = 2.0;
float SLIDE_MIN_ANGLE = 0.6;


// ------------------
// > FUNCTIONS
// ------------------

// main functions
EntitiesCollision        CL_check_collision_horizontal(
      Player* player,
      EntityBufferElement* entity_iterator,
      int entity_list_size, bool iterative,
      Entity* skip_entity,
      bool dont_skip_if_inside_slope
); 
EntitiesCollision  CL_check_collision_vertical         (Player* player, EntityBufferElement* entity_iterator, int entity_list_size);

// helper functions
PrimitivesCollision      CL_get_vertical_overlap_player_vs_aabb          (Entity* entity, Entity* player);
PrimitivesCollision      CL_get_horizontal_overlap_with_player           (Entity* entity, Player* player);
EntitiesCollision        CL_get_terrain_height_at_player                 (Entity* player, Entity* entity);

RaycastTest    CL_check_for_floor_below_player                          (Player* player);
bool           CL_intersects_vertically_or_cull                         (Entity* entity, Player* player);
bool           CL_intersects_vertically_with_slope                      (Entity* slope, Entity* player);
bool           CL_player_feet_center_touches_slope                      (Player* player, Entity* slope);
bool           CL_check_event_trigger_collision                         (Entity* trigger, Entity* player);
bool           CL_player_qualifies_as_standing                          (Player* player);
bool           CL_player_hit_ceiling_slope                              (Player* player, Entity* slope);
float          CL_get_distance_from_slope                               (Entity* slope, Player* player);
float          CL_get_slope_height_at_position                          (vec3 position, Entity* slope);
float          CL_get_slope_height_at_position                          (float position, Entity* slope, bool is_x, bool is_z);
float          CL_get_slope_height_at_position_along_inclination_axis   (float position, Entity* slope);
auto           CL_project_entity_into_slope                             (Entity* entity, Entity* ramp);



bool CL_player_qualifies_as_standing(Player* player)
{
   // this serves only to enable us to check for standing_entity_ptr, otherwise its NULL and we get an exception
   return player->player_state   == PLAYER_STATE_STANDING      || 
      player->player_state       == PLAYER_STATE_SLIDING       ||
      player->player_state       == PLAYER_STATE_SLIDE_FALLING;
}


auto CL_project_entity_into_slope(Entity* entity, Entity* ramp)
{
   // projects the 3D bounding box into the axis-aligned ramp inclined line.
   // this way we can retrieve what is the height of the slope at the entity bounds.
   struct slope_heights{
      float min;
      float max;
   } heights;

   auto slope_c            = ramp->collision_geometry.slope;
   auto [x0, x1, z0, z1]   = entity->get_rect_bounds();
   float p0, p1;

   if(!is_zero(slope_c.tangent.x))
   {
      p0 = CL_get_slope_height_at_position(x0, ramp, true, false);
      p1 = CL_get_slope_height_at_position(x1, ramp, true, false);
   }
   else if(!is_zero(slope_c.tangent.z))
   {
      p0 = CL_get_slope_height_at_position(z0, ramp, false, true);
      p1 = CL_get_slope_height_at_position(z1, ramp, false, true);
   }
   else assert(false);

   heights.min = min(p0, p1);
   heights.max = max(p0, p1);

   return heights;
}


PrimitivesCollision CL_get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player)
{
   // assumes we already checked that we have vertical intersection 
   auto aabb = entity->collision_geometry.aabb;
   auto p_collision = player->collision_geometry.cylinder;

   if(player->position.y >= entity->position.y)
   {
      float box_top              = entity->position.y + aabb.height;
      float player_bottom        = player->position.y - p_collision.half_length;
      return PrimitivesCollision{true, box_top - player_bottom, vec2(0, 1)};
   }
   else
   {
      float box_bottom           = entity->position.y;
      float player_top           = player->position.y + p_collision.half_length;
      return PrimitivesCollision{true, player_top - box_bottom, vec2(0, -1)};
   }
}


bool CL_intersects_vertically_or_cull(Entity* entity, Player* player)
{
   // if we are standing in a slope, then we need to check which player height we want to check against,
   // because at each point that the player touches the slope he has a different height
   // @todo: shouldnt here we ask (player_qualifies_as_standing) ? If sliding, doesnt this matter too?
   if(CL_player_qualifies_as_standing(player) &&
      player->standing_entity_ptr->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
   {
      float box_height  = entity->get_height();
      float box_top     = entity->position.y + box_height;
      float box_bottom  = entity->position.y;
      float p_feet      = player->entity_ptr->position.y - player->half_height;
      float player_top  = player->entity_ptr->position.y + player->half_height;

      // basic y cull
      if(p_feet + COLLISION_EPSILON >= box_top || player_top - COLLISION_EPSILON <= box_bottom)
         return false;

      auto ramp = player->standing_entity_ptr;

      auto [p_min_y, p_max_y]     = CL_project_entity_into_slope(player->entity_ptr, ramp);
      auto [box_min_y, box_max_y] = CL_project_entity_into_slope(entity, ramp);

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

      float box_top = entity->position.y + entity->get_height();
      float box_bottom = entity->position.y;
      
      return player_bottom + COLLISION_EPSILON < box_top && player_top > box_bottom + COLLISION_EPSILON;
   }
}


PrimitivesCollision CL_get_horizontal_overlap_with_player(Entity* entity, Player* player)
{
   // If the player's center lies inside entity, returns that it is.
   // If not, but there is overlap, returns the actual overlap.

   //now this serves for slopes and aabbs too.
   auto [x0, x1, z0, z1] = entity->get_rect_bounds();

   float player_x = player->entity_ptr->position.x;
   float player_z = player->entity_ptr->position.z;  

   return CL_circle_vs_square(player_x, player_z, player->radius, x0, x1, z0, z1);
}



float CL_get_distance_from_slope(Entity* slope, Player* player)
{
   // assumes we already checked that we have vertical intersection 
   float y = CL_get_slope_height_at_position(player->entity_ptr->position, slope);
   float player_bottom = player->entity_ptr->position.y - player->half_height;
   return y - player_bottom;
}


bool CL_player_hit_ceiling_slope(Player* player, Entity* slope)
{
   // considering that player checks if player's head is between slope floor's and slope's ramp
   bool player_is_going_up = player->prior_position.y < player->entity_ptr->position.y;
   //float y = CL_get_slope_height_at_position(player->entity_ptr->position, slope);
   float p_head = player->top().y;
   float p_feet = player->feet().y;
   float y_diff = p_head - slope->position.y;
   return player_is_going_up && y_diff > 0 && y_diff <= 0.035; 
}


float CL_get_slope_height_at_position(vec3 position, Entity* slope)
{
   // this will check the slope orientation for us and do the right thing
   float height = -1;
   auto slope_c = slope->collision_geometry.slope;
   float slope_top = slope->position.y + slope_c.height;

   if (!is_zero(slope_c.tangent.x))
      height = slope_top - slope_c.inclination * slope_c.tangent.x * (position.x - slope->position.x);
   else if (!is_zero(slope_c.tangent.z))
      height = slope_top - slope_c.inclination * slope_c.tangent.z * (position.z - slope->position.z);
   else assert(false);

   return height; 
}


float CL_get_slope_height_at_position(float position, Entity* slope, bool is_x, bool is_z)
{
   // this will assume we checked the slope orientation before calling it
   float height      = -1;
   auto  slope_c     = slope->collision_geometry.slope;
   float slope_top   = slope->position.y + slope_c.height;

   if (is_x)
      height = slope_top - slope_c.inclination * slope_c.tangent.x * (position - slope->position.x);
   else if (is_z)
      height = slope_top - slope_c.inclination * slope_c.tangent.z * (position - slope->position.z);
   else assert(false);

   return height;
}


bool CL_intersects_vertically_with_slope(Entity* slope, Entity* player)
{
   float slope_top      = slope->position.y + slope->collision_geometry.slope.height;
   float slope_bottom   = slope->position.y;

   auto  p_col          = player->collision_geometry.cylinder;
   float player_bottom  = player->position.y - p_col.half_length;
   float player_top     = player->position.y + p_col.half_length;

   // basic cull in y (we use epsilon so if same height no collision is detected)
   if(player_bottom + COLLISION_EPSILON  >= slope_top || player_top - COLLISION_EPSILON <= slope_bottom)
      return false;

   // gets the player projection into the slope
   auto [p_min_y, p_max_y] = CL_project_entity_into_slope(player, slope);
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


float CL_get_vertical_overlap_player_slope(Entity* slope, Entity* player)
{
   // dont know what to do with this procedure yet
   float slope_top               = slope->position.y + slope->collision_geometry.slope.height;

   auto p_col                    = player->collision_geometry.cylinder;
   float player_bottom           = player->position.y - p_col.half_length;
   float player_top              = player->position.y + p_col.half_length;

   // gets the player projection into the slope
   auto [p_min_y, p_max_y]       = CL_project_entity_into_slope(player, slope);
   float diff_top                = slope_top - p_min_y;

   // player is not touching the ramp
   if(p_max_y < player_bottom) return -1;

   if(p_max_y > slope_top)     return slope_top - player_bottom;
   else                        return p_max_y - player_bottom;
}


EntitiesCollision CL_get_terrain_height_at_player(Entity* player, Entity* entity)
{
   EntitiesCollision cd;
   auto [x0, x1, z0, z1] = entity->get_rect_bounds();
   float height;

   switch(entity->collision_geometry_type)
   {
      case COLLISION_ALIGNED_BOX:
         height = entity->position.y + entity->collision_geometry.aabb.height;
         break;
      case COLLISION_ALIGNED_SLOPE:
         height = CL_get_slope_height_at_position(player->position, entity);
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


RaycastTest CL_check_for_floor_below_player(Player* player)
{
   // cast ray slightly above contact point to catch slope tunneling correctly
   // (we only get to this point with low inclination slopes, the other ones trigger collision before this point)
   float tunneling_tolerance = 0.03;

   auto downward_ray    = Ray{player->feet() + vec3{0.0f, tunneling_tolerance, 0.0f}, -UNIT_Y};
   RaycastTest raytest  = test_ray_against_scene(downward_ray, RayCast_TestOnlyFromOutsideIn, player->entity_ptr);

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


EntitiesCollision CL_check_collision_horizontal(
   Player* player, 
   EntityBufferElement* entity_iterator,
   int entity_list_size,
   bool iterative = true,
   Entity* skip_entity = NULL,
   bool dont_skip_if_inside_slope = false) 
{
   //PS: the iterative argument in this function should be true whenever we are doing iterative checks and need to mark as checked entities
   // that had a collision with player in a previous iteration

   EntitiesCollision return_cd; 

   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;
      
      // skip if match any
      bool entity_is_player            = entity->name == "Player",
           entity_is_player_ground     = CL_player_qualifies_as_standing(player) && player->standing_entity_ptr == entity,
           culled                      = !CL_intersects_vertically_or_cull(entity, player),
           checked                     = iterative && entity_iterator->collision_check,
           skip_it                     = skip_entity != NULL && skip_entity == entity;

      if(entity_is_player || entity_is_player_ground || culled || checked || skip_it)
      {
         entity_iterator++;
         continue;
      }

      // start test
      float biggest_overlap = -1;                        //@wtf why is this inside the loop?
      PrimitivesCollision collision;
      bool set_collided_entity = false;

      // AABB
      if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
      {
         collision = CL_get_horizontal_overlap_with_player(entity, player);

         if(collision.is_collided && collision.overlap > biggest_overlap)
         {
            set_collided_entity                       = true;
            return_cd.collision_outcome               = BLOCKED_BY_WALL;
         }
      }

      // ALIGNED SLOPE
      else if (entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
      {
         // get some info about slope
         auto col_geometry = entity->collision_geometry.slope;
         auto slope_2d_tangent = get_slope_normal(entity);

         collision = CL_get_horizontal_overlap_with_player(entity, player);

         // return early if not collided or player is inside slope (unless we set the flag to check this case)
         if(!collision.is_collided || (!dont_skip_if_inside_slope && (collision.is_inside || collision.overlap < biggest_overlap)))
         {
            entity_iterator++;
            continue;
         }

         // player is hitting slope like a wall
         if(player->feet().y < entity->position.y)
         {
            set_collided_entity                        = true;
            return_cd.collision_outcome                = BLOCKED_BY_WALL;
         }

         // player is facing slope inclined face
         else if(is_equal(collision.normal_vec, slope_2d_tangent))
         {
            // if slope is very inclined...
            if(CL_player_qualifies_as_standing(player) && col_geometry.inclination > SLIDE_MIN_ANGLE)
            {
               // ...then, we care if the player cylinder touches the slope in any way (cylinder tips count)
               if(CL_intersects_vertically_with_slope(entity, player->entity_ptr))
               {
                  set_collided_entity                 = true;
                  return_cd.collision_outcome         = BLOCKED_BY_WALL;
               }
            }
            else
            {
               // either player is (falling) OR (standing but slope is not angled enough)...
               // we should only care for collisions between slope and the player center in x-z (feet center)
               float v_overlap                        = CL_get_distance_from_slope(entity, player);
               if(v_overlap > 0)
               {
                  set_collided_entity                 = true;
                  return_cd.collision_outcome         = STEPPED_SLOPE;
                  collision.overlap                   = v_overlap; // @WORKAROUND
               }
            }
         }
         // player is not facing slope inclined face
         else
         {
            // ...then we care if player (as a cylinder) is touching the slope
            if(CL_intersects_vertically_with_slope(entity, player->entity_ptr))
            {
               set_collided_entity                    = true;
               return_cd.collision_outcome            = BLOCKED_BY_WALL;
            }
         }
      }

      // set current entity as collided one
      if(set_collided_entity)
      {
         // cout << "horizontal collision with '" << entity->name << "'\n";
         biggest_overlap = collision.overlap;

         return_cd.is_collided                        = true;
         return_cd.collided_entity_ptr                = entity;
         return_cd.overlap                            = collision.overlap;
         return_cd.normal_vec                         = collision.normal_vec;
      }

      entity_iterator++;
   }

   return return_cd;
}


EntitiesCollision CL_check_collision_vertical(Player* player, EntityBufferElement* entity_iterator, int entity_list_size)
{
   EntitiesCollision return_cd; 
   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;
      if(entity->name == "Player")
      {
         entity_iterator++;
         continue;
      }

	   float biggest_overlap = -1;                        //@wtf why is this inside the loop?
	   if (entity_iterator->collision_check == false)
      {   
         float vertical_overlap = -1; 
         PrimitivesCollision v_overlap_collision;                  // for ceilling hit detection
         PrimitivesCollision horizontal_check;

         // player cylinder doesnt intersect entity's bounding box    
         if(!CL_intersects_vertically_or_cull(entity, player))
         { 
            entity_iterator++; 
            continue; 
         }
         
         // A) CHECKS ENTITY GEOMETRIC TYPE
         switch(entity->collision_geometry_type)
         {
            case COLLISION_ALIGNED_BOX:
               v_overlap_collision           = CL_get_vertical_overlap_player_vs_aabb(entity, player->entity_ptr);
               vertical_overlap              = v_overlap_collision.overlap;
               horizontal_check              = CL_get_horizontal_overlap_with_player(entity, player);
               break;
            case COLLISION_ALIGNED_SLOPE:
            {
               auto top = entity->position.y + entity->get_height();
               if(top < player->feet().y)
               { 
                  entity_iterator++; 
                  continue; 
               }
               // here we should get tunneling
               vertical_overlap           = CL_get_distance_from_slope(entity, player);
               horizontal_check           = CL_get_horizontal_overlap_with_player(entity, player);   
               break;
            }
         }

         // B) CHECKS IF ANYTHING WORHTWHILE HAPPENED
         if(horizontal_check.is_collided && vertical_overlap >= 0 && vertical_overlap > biggest_overlap)
         {
            biggest_overlap = vertical_overlap;
            return_cd.collided_entity_ptr = entity;

            // player jumped and hit the ceiling (AABB)
            // @bug this is completely wrong! the normal vec being used is fake.
            // I cant get rid of this until we have real cylinder vs box collision detection in place.
            if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX &&
               v_overlap_collision.normal_vec.y == -1 &&
               player->prior_position.y < player->entity_ptr->position.y &&
               vertical_overlap > 0)
            {
               return_cd.overlap             = vertical_overlap;
               return_cd.normal_vec          = v_overlap_collision.normal_vec;
               return_cd.collision_outcome   = JUMP_CEILING;
            }

            // player jumped and hit the ceiling (SLOPE)
            else if( entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE &&
                     CL_player_hit_ceiling_slope(player, entity))
            {
               return_cd.overlap             = player->top().y - entity->position.y;
               return_cd.normal_vec          = vec2(0, -1);
               return_cd.collision_outcome   = JUMP_CEILING;
            }

            // player fell inside a slope
            else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE &&
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

            // player has part of body outside the entity bounds
            // and is overlapping too much (from top of entity to player feet)
            else if(!horizontal_check.is_inside && vertical_overlap > 0.01) 
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


bool CL_check_event_trigger_collision(Entity* trigger, Entity* player)
{
   auto col2                  = player->collision_geometry.cylinder;
   auto top1                  = trigger->position.y + trigger->trigger_scale.y * 2;
   auto top2                  = player->position.y + col2.half_length * 2;
   auto bottom1               = trigger->position.y;
   auto bottom2               = player->position.y;

   if(top1 < bottom2 || bottom1 > top2)
      return false;
   
   auto trigger_pos_2d        = vec2{trigger->trigger_pos.x, trigger->trigger_pos.z};
   auto player_pos_2d         = vec2{player->position.x, player->position.z};
   auto dist_2d               = glm::length(trigger_pos_2d - player_pos_2d);
   auto radius_sum            = trigger->trigger_scale.x + col2.radius;

   if(radius_sum < dist_2d)
      return false;

   return true;
}


void CL_snap_player(Player* player, vec2 dir, float overlap)
{
   // this will snap player to the collided entity according to its normal vec (if player overlaps it in the test)
   // it only does so in the XZ plane
   if(overlap > 0)
      player->entity_ptr->position -= to3d_xz(dir) * overlap;
}


vec3 CL_player_future_pos_obstacle(Player* player,  Entity* entity, vec2 normal_vec, float overlap)
{
   // returns the position of the player if he were to cross an obstacle (ledge grabbing standing or vaulting)

   // snap to entity wall and "go through it" in xz
   vec3 original_pos = player->entity_ptr->position;
   CL_snap_player(player, normal_vec, overlap);
   vec3 position = player->entity_ptr->position - to3d_xz(normal_vec) * player->radius * 2.f;

   // computes y position at that xz position
   float y_pos;
   if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
   {
      y_pos = entity->position.y + entity->get_height() + player->half_height;
      cout << entity->name << "'s height is: " << to_string(y_pos) << "\n";
   }
   else if (entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
      y_pos = CL_get_slope_height_at_position(position, entity) + player->half_height;
   else assert(false);
   position.y = y_pos;
   player->entity_ptr->position = original_pos;
   return position;
}


bool CL_test_in_mock_position(Player* player, vec3 pos, Entity* skip_entity = NULL)
{
   vec3 original_pos = player->entity_ptr->position;

   player->entity_ptr->position = pos;
   EntitiesCollision test = CL_check_collision_horizontal(
      player,
      G_BUFFERS.entity_buffer->buffer,
      G_BUFFERS.entity_buffer->size,
      false,
      skip_entity,
      true
   );

   player->entity_ptr->position = original_pos;
   return test.is_collided;
}
