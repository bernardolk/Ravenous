// ----------------------
// > CL_IgnoreColliders
// ----------------------
/* used, currently, to avoid detecting collision with colliders that are 
    being used by player as a floor platform / terrain
*/
// struct CL_IgnoreColliders {
//    const static size_t size = 5;
//    Entity* list[size] = {};
//    size_t count = 0;

//    void add(Entity* entity)
//    {
//       if(count == size)
//       {
//          cout << "CL_IgnoreColliders is full\n";
//          assert(false);
//       }

//       for(int i = 0; i < count; i++)
//          if(list[i] == entity)
//             return;

//       list[count] = entity;
//       count++;
//    };

//    void empty()
//    {
//       for(int i = 0; i < size; i++)
//          list[i] = NULL;
      
//       count = 0;
//    }

//    void remove(Entity* entity)
//    {
//       for(int i = 0; i < count; i++)
//          if(list[i] == entity)
//          {
//             list[i] = NULL;
//             for(int j = i; j < count - 1; j++)
//             {
//                list[j] = list[j + 1];
//                list[j + 1] = NULL;
//             }
//             count--;
//             return;
//          }
//    }

//    bool is_in_list(Entity* entity)
//    {
//       for(int i = 0; i < count; i++)
//          if(list[i] == entity)
//             return true;

//       return false;
//    }

// } CL_Ignore_Colliders;

#include <cl_gjk.h>
#include <cl_epa.h>
#include <cl_log.h>
#include <cl_resolvers.h>

// PROTOTYPES
CL_ResultsArray CL_test_and_resolve_collisions(Player* player);
CL_Results CL_test_collision_buffer_entitites(
   Player* player,
   EntityBufferElement* entity_iterator,
   int entity_list_size,
   bool iterative
);
CL_Results CL_test_player_vs_entity(Entity* entity, Player* player);
void CL_resolve_collision(CL_Results results, Player* player);
bool CL_test_collisions(Player* player);


// --------------------------------------
// > RUN INTERATIVE COLLISION DETECTION
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
   auto entity_buffer = G_BUFFERS.entity_buffer;
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
         CL_log_collision(result, c);
         CL_resolve_collision(result, player);
         results_array.results[results_array.count] = result;
         results_array.count++;
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
   auto entity_buffer = G_BUFFERS.entity_buffer;
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

   if(b_gjk && b_epa)
      RENDER_MESSAGE("COMPLETE COLLISION", 1000);
   else if (b_gjk)
      RENDER_MESSAGE("EPA UNRESOLVED COLLISION", 1000);

   return cl_results;
}