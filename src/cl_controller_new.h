struct CL_Results {
   bool collision = false;
   Entity* entity;
   float penetration;
   vec3 normal;
};

#include<cl_gjk.h>
#include<cl_epa.h>
#include<cl_resolvers_new.h>

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
         // CL_log_collision(result, c);
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
           entity_is_player_ground     = CL_player_qualifies_as_standing(player) && player->standing_entity_ptr == entity,
           checked                     = iterative && entity_iterator->collision_check,
           skip_it                     = skip_entity != NULL && skip_entity == entity;

      if(entity_is_player || entity_is_player_ground || checked || skip_it)
      {
         entity_iterator++;
         continue;
      }

      // here should test for bounding box collision (or any geometric first pass test) FIRST, then do the call below

      auto result = CL_test_player_vs_entity(entity, player);

      if(result.collision)
         return result;
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

   if(entity->name == "boxA")
      IM_RENDER.add_mesh(IMHASH, entity_collider);

   GJK_Result box_gjk_test = CL_run_GJK(entity_collider, player_collider);
   
   if(box_gjk_test.collision)
   {
      RENDER_MESSAGE("GJK COLLISION");
      EPA_Result epa = CL_run_EPA(box_gjk_test.simplex, entity_collider, player_collider);
      
      if(epa.collision)
      {
         // unstuck player
         player_entity->position += epa.direction * epa.penetration;

         cl_results.penetration = epa.penetration;
         cl_results.normal = epa.direction;
         cl_results.collision = true;
      }
   }

   return cl_results;
}