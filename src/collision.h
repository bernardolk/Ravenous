#include <algorithm>
#include <math.h>

const float COLLISION_EPSILON = 0.0001f;

enum CollisionType{
   HORIZONTAL = 0,
   VERTICAL = 1
};

enum CollisionOutcomeEnum {
   JUMP_SUCCESS      = 0,  // player fell succesfully into another platform
   JUMP_FAIL         = 1,  // player fell into the edge of another platform
   JUMP_FACE_FLAT    = 2,  // player fell with face flat on wall
   JUMP_SLIDE        = 3,
   JUMP_CEILING      = 4,
   STEPPED_SLOPE     = 5,
   BLOCKED_BY_WALL   = 6
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
CollisionData check_player_collision_with_scene_falling(
      Entity* player, Entity** entity_iterator, size_t entity_list_size
);
CollisionData check_collision_horizontal(
      Player* player, EntityBufferElement* entity_iterator, size_t entity_list_size
); 
CollisionData sample_terrain_height_below_player(Entity* player, Entity* entity); 
bool check_2D_collision_circle_and_aligned_square(Entity* circle, Entity* square);
float squared_distance_between_point_and_line(float x1, float y1, float x2, float y2, float x0, float y0);
float squared_minimum_distance(glm::vec2 v, glm::vec2 w, glm::vec2 p);
glm::vec3 get_nearest_edge(Entity* point, Entity* square);
bool intersects_vertically(Entity* entity, Player* player);
bool intersects_vertically_slope(Entity* entity, Entity* player);
bool intersects_vertically_standing_slope(Entity* entity, Player* player);
void run_collision_checks_standing(Player* player, Entity** entity_iterator, size_t entity_list_size);
void run_collision_checks_falling(Player* player, Entity** entity_iterator, size_t entity_list_size);
CollisionData check_collision_vertical(Player* player, EntityBufferElement* entity_iterator, size_t entity_list_size);
Collision get_horizontal_overlap_player_aabb(Entity* entity, Entity* player);
Collision get_horizontal_overlap_player_slope(Entity* entity, Entity* player);
float get_slope_inclination(Entity* entity);
float get_slope_height_at_player_position(Entity* player, Entity* entity);
Boundaries get_slope_boundaries(Entity* entity);
SlopeHeightsPlayer get_slope_heights_at_player(Entity* player, Entity* entity);
CollisionData check_for_floor_below_player(Player* player);



// ________________________________________________________________________________
//
// SCENE COLLISION CONTROLLER FUNCTIONS - ITERATIVE MULTI-CHECK COLLISION DETECTION 
// ________________________________________________________________________________

void run_collision_checks_standing(Player* player, Entity** entity_iterator, size_t entity_list_size)
{
   auto entity_buffer = (EntityBuffer*)G_BUFFERS.buffers[0];
   bool end_collision_checks = false;
   while(!end_collision_checks)
   {
      auto entity_buf_iter = entity_buffer->buffer;  // places pointer back to start
      CollisionData collision_data = check_collision_horizontal(player, entity_buf_iter, entity_list_size);
      if(collision_data.collided_entity_ptr != NULL)
      {
         // marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
         {
            entity_buf_iter = entity_buffer->buffer;
            for(int i = 0; i < entity_buffer->size; ++i)
            {
               if(entity_buf_iter->entity == collision_data.collided_entity_ptr)
               {
                  entity_buf_iter->collision_check = true;
                  break;
               }
               entity_buf_iter++;
            }
         }
         
         // deals with collision
         switch(collision_data.collision_outcome)
         {
            case STEPPED_SLOPE:
            {
               // cout << "PLAYER STEPPED INTO SLOPE \n";
               player->standing_entity_ptr = collision_data.collided_entity_ptr;
               player->entity_ptr->position.y += collision_data.overlap;
               // @TODO: this is weird, here we are kinda assuming the player is already standing,
               // (because we do not set its state to standing) and this is not that wrong since,
               // above, we already checked for vertical collisions and possibly dealt with... lost track of it. 
               break;
            }
            case BLOCKED_BY_WALL:
            {
               // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
               player->entity_ptr->position -= glm::vec3(
                  collision_data.normal_vec.x, 0, collision_data.normal_vec.y
               ) * collision_data.overlap;
               break;
            }
         }
      }
      else end_collision_checks = true;
   }
}

// @NOTE! : Because we are marking entities in buffer when checked, we should never use multiple CONTROLLER LEVEL calls unless
//          we reset the buffers to the active scene entity list
void run_collision_checks_falling(Player* player, Entity** entity_iterator, size_t entity_list_size)
{
   // Here, we will check first for vertical intersections to see if player needs to be teleported to the top of the entity
   // it has collided with. If so, we deal with it on the spot (teleports player) and then keep checking for other collisions
   // to be resolved once thats done. Horizontal checks come after vertical collisions because players shouldnt loose the chance
   // to make their jump because we are preventing them from getting stuck first.

   auto entity_buffer = (EntityBuffer*)G_BUFFERS.buffers[0];  
   bool end_collision_checks = false;
   while(!end_collision_checks)
   {
      bool any_collision = false;
      // CHECKS VERTICAL COLLISIONS
      {
         auto entity_iter = entity_buffer->buffer;  // places pointer back to start
         auto collision_data = check_collision_vertical(player, entity_iter, entity_list_size);
         if(collision_data.collided_entity_ptr != NULL)
         {
            any_collision = true;
            // marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
            {
               entity_iter = entity_buffer->buffer;
               for(int i = 0; i < entity_buffer->size; ++i)
               {
                  if(entity_iter->entity == collision_data.collided_entity_ptr)
                  {
                     entity_iter->collision_check = true;
                     break;
                  }
                  entity_iter++;
               }
            }
            
            // deals with collision
            switch(collision_data.collision_outcome)
            {
               case JUMP_FAIL:
               {
                  // make player "slide" towards edge and fall away from floor
                  std::cout << "FELL FROM EDGE" << "\n";
                  auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->entity_ptr->collision_geometry_ptr;
                  player->standing_entity_ptr = collision_data.collided_entity_ptr;
                  player->entity_ptr->position.y += collision_data.overlap; 
                  
                  // makes player move towards OUT of the platform
                  if(player->entity_ptr->velocity.x == 0 && player->entity_ptr->velocity.z == 0)
                  {
                     player->entity_ptr->velocity.x = -1 * collision_data.normal_vec.x * player->fall_from_edge_speed;
                     player->entity_ptr->velocity.z = -1 *collision_data.normal_vec.y * player->fall_from_edge_speed;
                  }
                  // makes player fall (combined movement in 3D, player for a moment gets "inside" the platform while he slips)
                  player->entity_ptr->velocity.y = - 1 * player->fall_speed;
                  
                  player->player_state = PLAYER_STATE_FALLING_FROM_EDGE;
                  break;
               }
               case JUMP_SUCCESS:
               {
                  std::cout << "LANDED" << "\n";
                  // move player to surface, stop player and set him to standing
                  player->standing_entity_ptr = collision_data.collided_entity_ptr;
                  auto height_check = sample_terrain_height_below_player(player->entity_ptr, player->standing_entity_ptr);
                  player->entity_ptr->position.y = height_check.overlap + player->half_height; 
                  player->entity_ptr->velocity = glm::vec3(0,0,0);
                  player->player_state = PLAYER_STATE_STANDING;
                  break;
               }
               case JUMP_FACE_FLAT:
               {
                  std::cout << "JUMP FACE FLAT" << "\n";
                  // deals with collision
                  // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
                  player->entity_ptr->position -= 
                        glm::vec3(collision_data.normal_vec.x, 0, collision_data.normal_vec.y)  * collision_data.overlap;

                  // make player slide through the tangent of platform
                  auto tangent_vec = glm::vec2(collision_data.normal_vec.y, collision_data.normal_vec.x);      
                  auto v_2d = glm::vec2(player->entity_ptr->velocity.x, player->entity_ptr->velocity.z);
                  auto project = (glm::dot(v_2d, tangent_vec)/glm::length2(tangent_vec))*tangent_vec;

                  player->entity_ptr->velocity.x = project.x;
                  player->entity_ptr->velocity.z = project.y; 
                       
                  if(player->player_state == PLAYER_STATE_JUMPING)
                  {
                     player->player_state = PLAYER_STATE_FALLING;
                     player->entity_ptr->velocity.y = 0;
                  }

                  break;
               }
               case JUMP_SLIDE:
               {
                  std::cout << "SLIDING" << "\n";
                  
                  player->standing_entity_ptr = collision_data.collided_entity_ptr;
                  auto height_check = sample_terrain_height_below_player(player->entity_ptr, player->standing_entity_ptr);
                  player->entity_ptr->position.y = height_check.overlap + player->half_height;
                  // make player 'snap' to slope
                  auto collision_geom = *((CollisionGeometrySlope*) collision_data.collided_entity_ptr->collision_geometry_ptr);
                  auto &pv = player->entity_ptr->velocity;
                  auto pv_2d = glm::vec2(pv.x, pv.z);
                  // make camera (player) turn to face either up or down the slope

                  // if(G_SCENE_INFO.view_mode == FIRST_PERSON)
                  // {
                  //    auto t_2d = glm::vec2(collision_geom.tangent.x, collision_geom.tangent.z);
                  //    auto dot = glm::dot(pv_2d, t_2d);
                  //    if(dot == 0) dot = 1;   // compensates for orthogonal v and tangent
                  //    auto projected = (dot/glm::length2(t_2d))*t_2d;
                  //    auto camera_dir = glm::vec3(projected.x, G_SCENE_INFO.camera->Front.y, projected.y);
                  //    camera_look_at(G_SCENE_INFO.camera, camera_dir, false);
                  // }

                  pv = player->slide_speed * collision_geom.tangent;
                  player->player_state = PLAYER_STATE_SLIDING;
               }
               case JUMP_CEILING:
               {
                  std::cout << "HIT CEILING" << "\n";
                  player->entity_ptr->position.y -= collision_data.overlap + COLLISION_EPSILON; 
                  player->player_state = PLAYER_STATE_FALLING;
                  player->entity_ptr->velocity.y = 0;
                  break; 
               }
            }
         }
      }

      // CHECKS HORIZONTAL COLLISIONS
      {
         auto entity_iter = entity_buffer->buffer;  // places pointer back to start
         auto collision_data = check_collision_horizontal(player, entity_iter, entity_list_size);
         if(collision_data.collided_entity_ptr != NULL)
         {
            any_collision = true;
            // marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
            {
               entity_iter = entity_buffer->buffer;
               for(int i = 0; i < entity_buffer->size; ++i)
               {
                  if(entity_iter->entity == collision_data.collided_entity_ptr)
                  {
                     entity_iter->collision_check = true;
                     break;
                  }
                  entity_iter++;
               }
            }
            
            // deals with collision
            switch(collision_data.collision_outcome)
            {
               case STEPPED_SLOPE:
               {
                   cout << "PLAYER STEPPED INTO SLOPE \n";
                  player->standing_entity_ptr = collision_data.collided_entity_ptr;
                  player->entity_ptr->position.y += collision_data.overlap;
                  // @TODO: this is weird, here we are kinda assuming the player is already standing,
                  // (because we do not set its state to standing) and this is not that wrong since,
                  // above, we already checked for vertical collisions and possibly dealt with... lost track of it. 
                  break;
               }
               case BLOCKED_BY_WALL:
               {
                  // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
                   player->entity_ptr->position -= glm::vec3(
                     collision_data.normal_vec.x, 0, collision_data.normal_vec.y
                  ) * collision_data.overlap;
                  break;
               }
            }
         }
      }
      if(!any_collision)
      {
         end_collision_checks = true;
      }
   }
}


// ______________________________________
//
//  ENTITY COLLISION CONTROLLER FUNCTIONS
// ______________________________________

CollisionData check_collision_horizontal(Player* player, EntityBufferElement* entity_iterator, size_t entity_list_size) 
{
   CollisionData return_cd; 
   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;
	   float biggest_overlap = -1;
      Collision c;
      bool set_collided_entity = false;     
	   if (entity_iterator->collision_check == false && 
         !(player->player_state == PLAYER_STATE_STANDING && player->standing_entity_ptr == entity))
      {    
         // AABB
         if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX &&
            intersects_vertically(entity, player))
         {
            c = get_horizontal_overlap_player_aabb(entity, player->entity_ptr);

            if(c.is_collided && c.overlap >= 0 && c.overlap > biggest_overlap)
            {
               return_cd.collision_outcome = BLOCKED_BY_WALL;
               set_collided_entity = true;
            }
         }

         // ALIGNED SLOPE
         else if (entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE &&
                  intersects_vertically_slope(entity, player->entity_ptr))
         {
            c = get_horizontal_overlap_player_slope(entity, player->entity_ptr);
            if(c.is_collided && c.overlap > biggest_overlap)
            {
               auto col_geometry = *((CollisionGeometrySlope*) entity->collision_geometry_ptr);
               auto slope_2d_tangent = glm::normalize(glm::vec2(col_geometry.tangent.x, col_geometry.tangent.z));

               set_collided_entity = true;

               if(player->player_state == PLAYER_STATE_STANDING &&
                  c.overlap == 0)
               {
                  return_cd.collision_outcome = STEPPED_SLOPE;
                  // @WORKAROUND
                  float v_overlap = get_vertical_overlap_player_vs_slope(entity, player->entity_ptr);
                  c.overlap = v_overlap;
               }

               else if(player->player_state == PLAYER_STATE_STANDING &&
                  c.overlap > 0 &&  // this means player is not INSIDE entity (player centroid)
                  compare_vec2(c.normal_vec, -1.0f * slope_2d_tangent))
               {
                  float inclination = get_slope_inclination(entity);
                  if(inclination > 0.6)
                  {
                     return_cd.collision_outcome = BLOCKED_BY_WALL;
                  }
               }
               else if(c.overlap > 0)
               {
                  return_cd.collision_outcome = BLOCKED_BY_WALL;
               }
            }
         }

         // set current entity as collided one
         if(set_collided_entity)
         {
            cout << "horizontal collision with '" << entity->name << "'\n";
            biggest_overlap = c.overlap;

            return_cd.is_collided = true;
            return_cd.collided_entity_ptr = entity;
            return_cd.overlap = c.overlap;
            return_cd.normal_vec = c.normal_vec;
         }
      }
      entity_iterator++;
   }
   return return_cd;
}


CollisionData check_collision_vertical(Player* player, EntityBufferElement* entity_iterator, size_t entity_list_size)
{
   CollisionData return_cd; 
   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;
	   float biggest_overlap = -1;
	   if (entity_iterator->collision_check == false)
      {   
         // PERFORMS OVERLAP TESTING
         float vertical_overlap = -1;
         float inclination = -1; // for slopes only BADBADNOTGOOD
         Collision v_overlap_collision; // for ceilling hit detection
         Collision horizontal_check;
         {
            if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX && intersects_vertically(entity, player))
            {
               v_overlap_collision = get_vertical_overlap_player_vs_aabb(entity, player->entity_ptr);
               vertical_overlap = v_overlap_collision.overlap;
               horizontal_check = get_horizontal_overlap_player_aabb(entity, player->entity_ptr);
            }
            else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE && intersects_vertically_slope(entity, player->entity_ptr))
            {
               vertical_overlap = get_vertical_overlap_player_vs_slope(entity, player->entity_ptr);
               horizontal_check = get_horizontal_overlap_player_slope(entity, player->entity_ptr);   
               inclination = get_slope_inclination(entity);   
            }
         }
         // CHECKS IF ANYTHING WORHTWHILE HAPPENED
         if(horizontal_check.is_collided && vertical_overlap >= 0 && vertical_overlap > biggest_overlap)
         {
            biggest_overlap = vertical_overlap;
            return_cd.collided_entity_ptr = entity;

            if(horizontal_check.overlap == 0 && v_overlap_collision.normal_vec.y != -1)
            {
               // means player have "entered" other entity (only possible with slopes)
               if(inclination > 0.6)
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

            else if(v_overlap_collision.normal_vec.y == -1)
            {
               // means player hit the ceiling
               return_cd.overlap = vertical_overlap;
               return_cd.normal_vec = v_overlap_collision.normal_vec;
               return_cd.collision_outcome = JUMP_CEILING;
            }

            else if(vertical_overlap > 0.001) 
            {
               // if here, means player did not make the jump (feet is too low)
               return_cd.overlap = horizontal_check.overlap;
               return_cd.normal_vec = horizontal_check.normal_vec;
               return_cd.collision_outcome = JUMP_FACE_FLAT;
            }
            else if(horizontal_check.overlap < player->radius)
            {
               // did not get half body inside platform (didnt make the jump)
               return_cd.overlap = vertical_overlap;
               return_cd.normal_vec = horizontal_check.normal_vec;
               return_cd.collision_outcome = JUMP_FAIL;
            }
            else
            {
               // got half body inside platform (made the jump)
               return_cd.overlap = vertical_overlap;
               return_cd.collision_outcome = JUMP_SUCCESS;
            }
         }
      }
      entity_iterator++;
   }
   return return_cd;
}

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

float get_slope_inclination(Entity* entity)
{
   auto col_geometry = *((CollisionGeometrySlope*) entity->collision_geometry_ptr);
   float a = col_geometry.slope_height / col_geometry.slope_length;
   return a;
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

   if(entity->name == "corner box 2")
   {
     auto a = 0;  
   }

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
   float min_distance = 0.2;  // CONTROLS MAX HEIGHT FOR PLAYER TO NOT FALL STRAIGHT WHEN QUITTING PLATFORM
   CollisionData response;
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator;

      auto check = sample_terrain_height_below_player(player->entity_ptr, entity);
      float y_diff = (player->entity_ptr->position.y - player->half_height) - check.overlap; //here overlap is height...
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


