#include <string>
#include <vector>
#include <engine/core/rvn_types.h>
#include <engine/rvn.h>
#include <rvn_macros.h>
//#include <engine/camera.h>
#include <engine/vertex.h>
#include <engine/collision/primitives/bounding_box.h>
#include <map>
#include <iostream>
#include <engine/mesh.h>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/triangle.h>
#include <glm/gtx/quaternion.hpp>
#include <engine/entity.h>
//#include <engine/collision/primitives/ray.h>
#include <engine/world/world.h>
#include <engine/rvn.h>
#include <engine/collision/simplex.h>
#include <engine/collision/cl_gjk.h>
#include <engine/collision/cl_epa.h>
#include <player.h>
#include <colors.h>
#include <engine/collision/cl_types.h>
#include <engine/collision/cl_resolvers.h>
#include <engine/collision/cl_controller.h>
// #include <cl_log.h>


// ----------------------------
// > UPDATE PLAYER WORLD CELLS   
// ----------------------------

bool CL_update_player_world_cells(Player* player, World* world)
{
   /* Updates the player's world cells
      Returns whether there were changes or not to the cell list
      @todo - the procedures invoked here seem to do more work than necessary. Keep this in mind.
   */ 
  
   auto update_cells = world->update_entity_world_cells(player->entity_ptr);
   if(!update_cells.status == CellUpdate_OK)
   {
      std::cout << update_cells.message << "\n";
      return false;
   }

   return update_cells.entity_changed_cell;
}


// ------------------------------
// > COLLISION BUFFER FUNCTIONS
// ------------------------------

void CL_recompute_collision_buffer_entities(Player* player)
{
   // copies collision-check-relevant entity ptrs to a buffer
   // with metadata about the collision check for the entity
   auto  collision_buffer  = RVN::entity_buffer->buffer;
   int   entity_count      = 0;
   for(int i = 0; i < player->entity_ptr->world_cells_count; i++)
   {
      auto cell = player->entity_ptr->world_cells[i];
      for(int j = 0; j < cell->count; j++)
      {
         auto entity = cell->entities[j];
         
         // adds to buffer only if not present already
         bool present = false;
         for(int k = 0; k < entity_count; k++)
            if(collision_buffer[k].entity->id == entity->id)
            {
               present = true;
               break;
            }

         if(!present)
         {
            collision_buffer[entity_count].entity           = entity;
            collision_buffer[entity_count].collision_check  = false;
            entity_count++;
            if(entity_count > RVN::COLLISION_BUFFER_CAPACITY) assert(false);
         }
      }
   }

   RVN::entity_buffer->size = entity_count;
}


void CL_reset_collision_buffer_checks()
{
   for(int i = 0; i < RVN::entity_buffer->size; i++)
      RVN::entity_buffer->buffer[i].collision_check = false;
}


void CL_mark_entity_checked(Entity* entity)
{
   // marks entity in entity buffer as checked so we dont check collisions for this entity twice (nor infinite loop)
   auto entity_buffer         = RVN::entity_buffer;
   auto entity_element        = entity_buffer->buffer;
   for(int i = 0; i < entity_buffer->size; ++i)
   {
      if(entity_element->entity == entity)
      {
         entity_element->collision_check = true;
         break;
      }
      entity_element++;
   }
}

// --------------------------------------
// > RUN ITERATIVE COLLISION DETECTION
// --------------------------------------
/* Current strategy looks like this:
   - We have a collision buffer which holds all entities currently residing inside player's current world cells.
   - We iterate over these entities and test them one by one, if we encounter a collision, we resolve it and
      mark that entity as checked for this run.
   - Player state is changed accordingly, in this step. Not sure if that is a good idea or not. Probably not.
      Would be simpler to just unstuck player and update and then change player state as a final step.
   - We then run the tests again, so to find new collisions at the player's new position.
   - Once we don't have more collisions, we stop checking.
*/

CL_ResultsArray CL_test_and_resolve_collisions(Player* player)
{
   // iterative collision detection
   auto results_array = CL_ResultsArray();
   auto entity_buffer = RVN::entity_buffer;
   int c = -1;
   while(true)
   {
      c++;
      // places pointer back to start
      auto buffer = entity_buffer->buffer;
      auto result = CL_test_collision_buffer_entitites(player, buffer, entity_buffer->size, true);

      if(result.collision)
      {
         CL_mark_entity_checked(result.entity);
         // @todo delete later!
         // if(!result.entity->dodged)
         // {
         //    if(result.entity->name == "missile")
         //    {
         //       RVN::print_dynamic("Player took damage!", 2000, COLOR_RED_2);
         //       RVN::print_dynamic("Missile exploded!", 2000, COLOR_YELLOW_1);
         //       Exploded = true;
         //    }

         //    CL_log_collision(result, c);   
         //    CL_resolve_collision(result, player);
         //    results_array.results[results_array.count] = result;
         //    results_array.count++;
         // }
         
      }
      else break;
   }
   CL_reset_collision_buffer_checks();

   return results_array;
}


bool CL_test_collisions(Player* player)
{
   // iterative collision detection
   bool any_collision = false;
   auto entity_buffer = RVN::entity_buffer;
   while(true)
   {
      // places pointer back to start
      auto buffer = entity_buffer->buffer;
      auto result = CL_test_collision_buffer_entitites(player, buffer, entity_buffer->size, true);

      if(result.collision)
      {
         CL_mark_entity_checked(result.entity);
         any_collision = true;
      }
      else 
         break;
   }
   CL_reset_collision_buffer_checks();

   return any_collision;
}

// ---------------------------
// > RUN COLLISION DETECTION
// ---------------------------

CL_Results CL_test_collision_buffer_entitites(
   Player* player,
   EntityBufferElement* entity_iterator,
   int entity_list_size,
   bool iterative = true)
{

   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;

      bool entity_is_player            = entity->name == "Player",
           checked                     = iterative && entity_iterator->collision_check;

      if(entity_is_player || checked)
      {
         entity_iterator++;
         continue;
      }

      // @todo - here should test for bounding box collision (or any geometric first pass test) 
      //          FIRST, then do the call below

      auto result = CL_test_player_vs_entity(entity, player);

      if(result.collision)
         return result;
      
      entity_iterator++;
   }

   return {};
}

// -------------------------
// > TEST PLAYER VS ENTITY
// -------------------------

CL_Results CL_test_player_vs_entity(Entity* entity, Player* player)
{
   CL_Results cl_results;
   cl_results.entity = entity;

   Entity* player_entity = player->entity_ptr;

   Mesh* entity_collider = &entity->collider;
   Mesh* player_collider = &player_entity->collider;

   GJK_Result box_gjk_test = CL_run_GJK(entity_collider, player_collider);
   
   bool b_gjk = false;
   bool b_epa = false;
   if(box_gjk_test.collision)
   {
      b_gjk = true;
      EPA_Result epa = CL_run_EPA(box_gjk_test.simplex, entity_collider, player_collider);
      
      if(epa.collision)
      {
         b_epa = true;
         cl_results.penetration  = epa.penetration;
         cl_results.normal       = epa.direction;
         cl_results.collision    = true;
      }
   }

   return cl_results;
}