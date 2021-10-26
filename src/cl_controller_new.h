#include<cl_gjk.h>
#include<cl_epa.h>
#include<cl_resolvers_new.h>


void CL_run_collision_detection(Entity* entityA, Player* player)
{
   Entity* player_entity = player->entity_ptr;
   Mesh box_collider_A = CL_get_collider(entityA);
   Mesh box_collider_B = CL_get_collider(player_entity);
   GJK_Result box_gjk_test = CL_run_GJK(&box_collider_A, &box_collider_B);
   
   
   IM_ED_toggle_btn(&IM_Values.btn2, "Unstuck");

   if(box_gjk_test.collision)
   {
      EPA_Result epa = CL_run_EPA(box_gjk_test.simplex, &box_collider_A, &box_collider_B);
      
      if(epa.collision)
      {
         RENDER_MESSAGE("Penetration: " + format_float_tostr(epa.penetration, 4), 1);

         RENDER_MESSAGE(to_str(player_entity->position), 1);

         float old_z = player_entity->position.z;
         player_entity->position += epa.direction * epa.penetration;
         CL_wall_slide_player(player, epa);

         player_entity->update();
      }
   }
}