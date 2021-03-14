#include <algorithm>
#include <math.h>

enum CollisionType{
   HORIZONTAL = 0,
   VERTICAL = 1
};

enum CollisionOutcomeEnum {
   JUMP_SUCCESS   = 0,  // player fell succesfully into another platform
   JUMP_FAIL      = 0,  // player fell into the edge of another platform
}

struct CollisionData {
   bool is_collided            = false;
   Entity* collided_entity_ptr = NULL;
   float overlap               = 0;
   glm::vec2 normal_vec        = glm::vec2(0,0);
   CollisionOutcomeEnum collision_outcome;
};

struct Collision {
   bool is_collided = false;
   float overlap;
   glm::vec2 normal_vec;
};


Collision get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player);
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
Collision get_horizontal_overlap_player_aabb(Entity* entity, Entity* player);
bool intersects_vertically(Entity* entity, Entity* player);
void run_collision_checks_standing(Player* player, Entity** entity_iterator, size_t entity_list_size);



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
      entity_buf_iter = entity_buffer->buffer;  // places pointer back to start
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
         {
            // move player back using aabb surface normal vec and computed player/entity overlap in horizontal plane
             player->entity_ptr->position -= glm::vec3(collision_data.normal_vec.x, 0, collision_data.normal_vec.y)  * collision_data.overlap;
         }
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
      // CHECKS VERTICAL COLLISIONS
      {
         auto entity_iter = entity_buffer->buffer;  // places pointer back to start
         auto collision_data = check_collision_vertical(player, entity_iter, entity_list_size);
         if(collision_data.collided_entity_ptr != NULL)
         {
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
            player->entity_ptr->position -= glm::vec3(collision_data.normal_vec.x, 0, collision_data.normal_vec.y)  * collision_data.overlap;
         }
      }

      // CHECKS HORIZONTAL COLLISIONS
      {
         auto entity_iter = entity_buffer->buffer;  // places pointer back to start
         auto collision_data = check_collision_horizontal(player, entity_iter, entity_list_size);
         if(collision_data.collided_entity_ptr != NULL)
         {
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
            player->entity_ptr->position -= glm::vec3(collision_data.normal_vec.x, 0, collision_data.normal_vec.y)  * collision_data.overlap;
         }
      }
      else end_collision_checks = true;
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
	   if (entity_iterator->collision_check == false
         // ^^^ entities in entity_buf_iter have metadata set to true once checked for collisions in outer loop
         && entity->collision_geometry_type == COLLISION_ALIGNED_BOX 
         && entity->name != player->standing_entity_ptr->name // TODO : might be unnecessary
         && intersects_vertically(entity, player->entity_ptr))
      {    
         c = get_horizontal_overlap_player_aabb(entity, player->entity_ptr);
         if(c.is_collided && c.overlap > biggest_overlap)
         {
            biggest_overlap = c.overlap;

            return_cd.collided_entity_ptr = entity;
            return_cd.overlap = c.overlap;
            return_cd.normal_vec = glm::normalize(c.normal_vec);
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
      Collision c;   
	   if (entity_iterator->collision_check == false
         // ^^^ entities in entity_buf_iter have metadata set to true once checked for collisions in outer loop
         && entity->collision_geometry_type == COLLISION_ALIGNED_BOX 
         && entity->name != player->standing_entity_ptr->name
         && intersects_vertically(entity, player->entity_ptr))
      {    
         c = get_horizontal_overlap_player_aabb(entity, player->entity_ptr);
         if(c.is_collided && c.overlap > biggest_overlap)
         {
            biggest_overlap = c.overlap;

            return_cd.collided_entity_ptr = entity;
            return_cd.overlap = c.overlap;
            return_cd.normal_vec = glm::normalize(c.normal_vec);
         }
      }
      entity_iterator++;
   }
   return return_cd;
}


CollisionData 
check_player_collision_with_scene_falling(Entity* player, Entity** entity_iterator, size_t entity_list_size) 
{
   // here the player is falling, which means in frame 0 he is higher in y than in frame 1
   // So, first, we check for vertical collisions: if the player is colliding enough with an
   // object, we can consider him standing on it first.
   // Secondly, we check for horizontal collision, so if the player cannot be considered standing,
   // we will move him away from the object just like on player standing collision check routine.
   CollisionData return_cd; 
   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = *entity_iterator;
	   float biggest_vertical_overlap = -1;
	   if (entity->collision_geometry_type == COLLISION_ALIGNED_BOX
         && intersects_vertically(entity, player))
	   {
         Collision v_test = get_vertical_overlap_player_vs_aabb(entity, player);
         if(v_test.is_collided
            && v_test.overlap > biggest_vertical_overlap)
         {
            biggest_vertical_overlap = v_test.overlap;

            return_cd.collided_entity_ptr = entity;
            return_cd.overlap = v_test.overlap;
            return_cd.collision_outcome = JUMP_SUCCESS;
         }
         else
         {
            Collision h_test = get_horizontal_overlap_player_aabb(entity, player);
            if(h_test.is_collided)
            {
               // TODO: this wont work when we have aabb's with height != 0 (not platforms, but solid 3D boxes or walls)
               //       because in that case, the player will in fact collide face-against it and not 'stand then fall from edge'
               return_cd.collided_entity_ptr = entity;
               return_cd.overlap = h_test.overlap;
               return_cd.collision_outcome = JUMP_FAIL;
               return_cd.normal_vec = glm::normalize(h_test.normal_vec);
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

   float nx = std::max(box_x0, std::min(box_x1, player_x));
   float nz = std::max(box_z0, std::min(box_z1, player_z));
   // vector from player to nearest point in rectangle surface
   glm::vec2 n_vec = glm::vec2(nx, nz) - glm::vec2(player_x, player_z);
   float distance = glm::length(n_vec);
   float p_radius = player_collision_geometry->radius;

   Collision c;
   if(p_radius > distance)
   {   
      c.is_collided = true;
      c.overlap = p_radius - distance;
      c.normal_vec = glm::normalize(n_vec);
   }

   return c;
}


Collision get_vertical_overlap_player_vs_aabb(Entity* entity, Entity* player)
{
   auto player_collision_geometry = (CollisionGeometryAlignedCylinder*) player->collision_geometry_ptr;
   auto box_collision_geometry = *((CollisionGeometryAlignedBox*) entity->collision_geometry_ptr);

   float player_x = player->position.x;
   float player_z = player->position.z;

   //box boundaries
   float box_x0 = entity->position.x;
   float box_x1 = entity->position.x + box_collision_geometry.length_x;
   float box_z0 = entity->position.z;
   float box_z1 = entity->position.z + box_collision_geometry.length_z;

   float nx = std::max(box_x0, std::min(box_x1, player_x));
   float nz = std::max(box_z0, std::min(box_z1, player_z));
   // vector from player to nearest point in rectangle surface
   glm::vec2 n_vec = glm::vec2(nx, nz) - glm::vec2(player_x, player_z);
   float distance = glm::length(n_vec);
   float p_radius = player_collision_geometry->radius;
   float horizontal_overlap = p_radius - distance;
   
   
   // Checks if player has half the body inside the platform to stand on OR
   // if his centroid lies inside the aabb rectangle (TODO: refactor this into small point_in_rect procedure)
   Collision c;
   if(horizontal_overlap > p_radius
      || (box_x0 <= player_x && box_x1 >= player_x && box_z0 <= player_z && box_z1 >= player_z))     
   {
      float overlap = player->position.y - box_top;
      float overlap_norm = overlap < 0 ? overlap * -1: overlap;
      Collision c;
      c.is_collided = true;
      c.overlap = overlap_norm;
      return c;
   }

   return Collision {false};
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

   return player_bottom <= box_top && player_top >= box_bottom;
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
      cd.overlap = box_top;
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


