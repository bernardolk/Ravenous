struct CL_Results {
   bool collision = false;
   Entity* entity;
   float penetration;
   vec3 normal;
};

// used, currently, to avoid detecting collision with colliders that are being used by player as a
// floor platform / terrain
struct CL_IgnoreColliders {
   const static size_t size = 5;
   Entity* list[size] = {};
   size_t count = 0;

   void add(Entity* entity)
   {
      if(count == size)
      {
         cout << "CL_IgnoreColliders is full\n";
         assert(false);
      }

      for(int i = 0; i < count; i++)
         if(list[i] == entity)
            return;

      list[count] = entity;
      count++;
   };

   void empty()
   {
      for(int i = 0; i < size; i++)
         list[i] = NULL;
      
      count = 0;
   }

   void remove(Entity* entity)
   {
      for(int i = 0; i < count; i++)
         if(list[i] == entity)
         {
            list[i] = NULL;
            for(int j = i; j < count - 1; j++)
            {
               list[j] = list[j + 1];
               list[j + 1] = NULL;
            }
            count--;
            return;
         }
   }

   bool is_in_list(Entity* entity)
   {
      for(int i = 0; i < count; i++)
         if(list[i] == entity)
            return true;

      return false;
   }

} CL_Ignore_Colliders;

#include <cl_gjk.h>
#include <cl_epa.h>
#include <cl_log.h>
#include <cl_resolvers_new.h>

// prototypes
void CL_run_iterative_collision_detection(Player* player);
CL_Results CL_run_collision_detection(
   Player* player,
   EntityBufferElement* entity_iterator,
   int entity_list_size,
   bool iterative ,
   Entity* skip_entity
);
CL_Results CL_test_player_vs_entity(Entity* entity, Player* player);
void CL_new_resolve_collision(CL_Results results, Player* player);


// functions

void CL_run_iterative_collision_detection(Player* player)
{
   // iterative collision detection
   auto entity_buffer = G_BUFFERS.entity_buffer;
   int c = -1;
   while(true)
   {
      c++;
      auto buffer = entity_buffer->buffer;            // places pointer back to start
      // TEMP - DELETE LATER
      auto result = CL_run_collision_detection(player, buffer, entity_buffer->size, true, player->skip_collision_with_floor);

      if(result.collision)
      {
         CL_mark_entity_checked(result.entity);
         CL_log_collision(result, c);
         CL_new_resolve_collision(result, player);
      }
      else break;
   }
}


CL_Results CL_run_collision_detection(
   Player* player,
   EntityBufferElement* entity_iterator,
   int entity_list_size,
   bool iterative = true,
   Entity* skip_entity = NULL)
{

   for (int i = 0; i < entity_list_size; i++)
   {
	   Entity* &entity = entity_iterator->entity;

      bool entity_is_player            = entity->name == "Player",
           checked                     = iterative && entity_iterator->collision_check,
           skip_it                     = skip_entity != NULL && skip_entity == entity;
           skip_it                     = skip_it || CL_Ignore_Colliders.is_in_list(entity);

      if(entity_is_player || checked || skip_it)
      {
         entity_iterator++;
         continue;
      }

      // here should test for bounding box collision (or any geometric first pass test) FIRST, then do the call below

      auto result = CL_test_player_vs_entity(entity, player);

      if(result.collision)
      {
         // unstuck player
         player->entity_ptr->position += result.normal * result.penetration;
         player->entity_ptr->update();
         return result;
      }
      
      entity_iterator++;
   }

   return {};
}


CL_Results CL_test_player_vs_entity(Entity* entity, Player* player)
{
   CL_Results cl_results;
   cl_results.entity = entity;

   Entity* player_entity = player->entity_ptr;

   Mesh* entity_collider = &entity->collider;
   Mesh* player_collider = &player_entity->collider;

   GJK_Result box_gjk_test = CL_run_GJK(entity_collider, player_collider);
   
   if(box_gjk_test.collision)
   {
      RENDER_MESSAGE("GJK COLLISION", 1000);
      EPA_Result epa = CL_run_EPA(box_gjk_test.simplex, entity_collider, player_collider);
      
      if(epa.collision)
      {
         cl_results.penetration  = epa.penetration;
         cl_results.normal       = epa.direction;
         cl_results.collision    = true;
      }
   }

   return cl_results;
}