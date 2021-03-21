#include <algorithm>
#include <math.h>

const float COLLISION_EPSILON = 0.0001f;

enum CollisionType{
   HORIZONTAL = 0,
   VERTICAL = 1
};

enum CollisionOutcomeEnum {
   JUMP_SUCCESS   = 0,  // player fell succesfully into another platform
   JUMP_FAIL      = 1,  // player fell into the edge of another platform
   JUMP_FACE_FLAT = 2,  // player fell with face flat on wall
   JUMP_SLIDE     = 3
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


float get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player);
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
bool intersects_vertically(Entity* entity, Entity* player);
void run_collision_checks_standing(Player* player, Entity** entity_iterator, size_t entity_list_size);
void run_collision_checks_falling(Player* player, Entity** entity_iterator, size_t entity_list_size);
CollisionData check_collision_vertical(Player* player, EntityBufferElement* entity_iterator, size_t entity_list_size);
Collision get_horizontal_overlap_player_aabb(Entity* entity, Entity* player);
Collision get_horizontal_overlap_player_slope(Entity* entity, Entity* player);
float get_slope_inclination(Entity* entity);
float get_slope_height_at_player_position(Entity* player, Entity* entity);
Boundaries get_slope_boundaries(Entity* entity);


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
         // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
            player->entity_ptr->position -= 
                  glm::vec3(collision_data.normal_vec.x, 0, collision_data.normal_vec.y)  * collision_data.overlap;
      }
      else end_collision_checks = true;
   }
}

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
                  // else
                  // {
                  //    player->entity_ptr->velocity = player->entity_ptr->velocity * player->fall_from_edge_speed;
                  // }

                  // makes player fall (combined movement in 3D, player for a moment gets "inside" the platform while he slips)
                  player->entity_ptr->velocity.y = - 1 * player->fall_speed;
                  
                  player->player_state = PLAYER_STATE_FALLING_FROM_EDGE;
                  break;
               }
               case JUMP_SUCCESS:
               {
                  std::cout << "LANDED" << "\n";
                  // move player to surface, stop player and set him to standing
                  auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->entity_ptr->collision_geometry_ptr;
                  // TODO: having problems with floating point precision here when calculating player height after overlapping
                  // vertically with some platform, so for now we are not CORRECTING the players height but SETTING it to the
                  // sampled platform height. Not sure what will be the final form of this calculation.
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

                  auto collision_geom = *((CollisionGeometrySlope*) collision_data.collided_entity_ptr->collision_geometry_ptr);
                  player->entity_ptr->velocity = player->slide_speed * collision_geom.tangent;
                  player->player_state = PLAYER_STATE_SLIDING;
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
            // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
            player->entity_ptr->position -= glm::vec3(collision_data.normal_vec.x, 0, collision_data.normal_vec.y) * collision_data.overlap;
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
	   if (entity_iterator->collision_check == false)
      {    
         if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX && intersects_vertically(entity, player->entity_ptr))
         {
            c = get_horizontal_overlap_player_aabb(entity, player->entity_ptr);

            if(c.is_collided && c.overlap >= 0 && c.overlap > biggest_overlap)
            {
               biggest_overlap = c.overlap;

               return_cd.is_collided = true;
               return_cd.collided_entity_ptr = entity;
               return_cd.overlap = c.overlap;
               return_cd.normal_vec = c.normal_vec;
            }
         }
         // here we check for entity different then players current floor and above no because above intersects_vertically
         // suffice to discard it
         else if (entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE && entity != player->standing_entity_ptr)
         {
            auto vertical_overlap = get_vertical_overlap_player_vs_slope(entity, player->entity_ptr);
            c = get_horizontal_overlap_player_slope(entity, player->entity_ptr);
            float inclination = get_slope_inclination(entity);
            // check if player state is standing and he is stepping inside slope from the slope side
            // not from the sides of it (there you have walls)
            if(c.is_collided && 
               player->player_state == PLAYER_STATE_STANDING &&
               c.overlap > 0 &&
               inclination > 0.6)
            {
               biggest_overlap = c.overlap;

               return_cd.is_collided = true;
               return_cd.collided_entity_ptr = entity;
               return_cd.overlap = c.overlap;
               return_cd.normal_vec = c.normal_vec;
            }
            if(c.is_collided && 
               player->player_state == PLAYER_STATE_STANDING &&
               c.normal_vec == glm::vec2(0,0) &&
               inclination <= 0.6)
            {
               // controls wheter the player is blocked or not by the slope inclination
               cout << "PLAYER STEPPED INTO SLOPE \n";
               player->standing_entity_ptr = entity;
               player->entity_ptr->position.y += vertical_overlap;
            }
            else if(c.is_collided && c.overlap >= 0 && c.overlap > biggest_overlap && vertical_overlap > 0)
            {
               biggest_overlap = c.overlap;

               return_cd.is_collided = true;
               return_cd.collided_entity_ptr = entity;
               return_cd.overlap = c.overlap;
               return_cd.normal_vec = c.normal_vec;
            }
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
         float inclination = -1; // for slopes only
         Collision horizontal_check;
         {
            if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX && intersects_vertically(entity, player->entity_ptr))
            {
               vertical_overlap = get_vertical_overlap_player_vs_aabb(entity, player->entity_ptr);
               horizontal_check = get_horizontal_overlap_player_aabb(entity, player->entity_ptr);
            }
            else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
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
            // Compute player fall collision outcome to inform scene collision controller
            if(horizontal_check.overlap == 0)
            {
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
            else if(vertical_overlap > 0.001) // TODO: Experiment with this cutoff value, 
            // here we determine how much "feet below aabb's top" the player has to be to be considered bashing face first against the wall
            {
               return_cd.overlap = horizontal_check.overlap;
               return_cd.normal_vec = horizontal_check.normal_vec;
               return_cd.collision_outcome = JUMP_FACE_FLAT;
            }
            else if(horizontal_check.overlap < player->radius)
            {
               return_cd.overlap = vertical_overlap;
               return_cd.normal_vec = horizontal_check.normal_vec;
               return_cd.collision_outcome = JUMP_FAIL;
            }
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

   if (bounds.x0 <= player_x && bounds.x1 >= player_x && bounds.z0 <= player_z && bounds.z1 >= player_z)
   {
      return Collision{true}; // overlap = 0
   }
   else
   {
      float nx = std::max(bounds.x0, std::min(bounds.x1, player_x));
      float nz = std::max(bounds.z0, std::min(bounds.z1, player_z));
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


float get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player)
{
   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
   float box_top = entity->position.y + box_collision_geometry.length_y;

   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   float player_bottom = player->position.y - player_collision_geometry->half_length;

   // float overlap = player->position.y - box_top;
   // float overlap_norm = overlap < 0 ? overlap * -1: overlap;
   return box_top - player_bottom;
}

float get_vertical_overlap_player_vs_slope(Entity* entity, Entity* player)
{
   auto col_geometry = *((CollisionGeometrySlope*) entity->collision_geometry_ptr);
   float slope_top = entity->position.y + col_geometry.slope_height;

   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   float player_bottom = player->position.y - player_collision_geometry->half_length;

   float a = col_geometry.slope_height / col_geometry.slope_length;
   float y = get_slope_height_at_player_position(player, entity);

   return y - player_bottom;
}


// _____________________________________
//
// CULLING / UTILITY COLLISION FUNCTIONS
// _____________________________________

bool intersects_vertically(Entity* entity, Entity* player)
{
   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   float player_bottom = player->position.y - player_collision_geometry->half_length;
   float player_top = player->position.y + player_collision_geometry->half_length;

   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);
   float box_top = entity->position.y + box_collision_geometry.length_y;
   float box_bottom = entity->position.y;

   return player_bottom < box_top && player_top > box_bottom;
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

   if(G_INPUT_INFO.key_input_state & KEY_P)
   {
      cout << entity->name << ": " << std::to_string(y) << "\n";
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
      assert(false);
   }

   float player_x = player->position.x;
   float player_z = player->position.z;

   CollisionData cd;
   // check if player's center lies inside terrain box
   if(x0 <= player_x && x1 >= player_x && z0 <= player_z && z1 >= player_z)
   {
      cd.overlap = height;
      cd.is_collided = true;
   }
   return cd;
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


