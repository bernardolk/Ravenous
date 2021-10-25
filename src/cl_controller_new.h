#include<cl_gjk.h>
#include<cl_epa.h>


void CL_run_collision_detection(Entity* entityA, Entity* entityB)
{
   Mesh box_collider_A = CL_get_collider(entityA);
   Mesh box_collider_B = CL_get_collider(entityB);
   GJK_Result box_gjk_test = CL_run_GJK(&box_collider_A, &box_collider_B);
   
   
   IM_ED_toggle_btn(&IM_Values.btn2, "Unstuck");

   if(box_gjk_test.collision)
   {
      // if(G_SCENE_INFO.tmp_unstuck_things)
      // {
         EPA_Result box_epa_test = CL_run_EPA(box_gjk_test.simplex, &box_collider_A, &box_collider_B);
         
         if(box_epa_test.collision)
         {
            RENDER_MESSAGE("Penetration: " + format_float_tostr(box_epa_test.penetration, 2), 1);

            RENDER_MESSAGE(to_str(entityB->position), 1);

            if(IM_Values.btn2)
            {
               entityB->position += box_epa_test.direction * box_epa_test.penetration;
               RENDER_MESSAGE("STUCK OUT: " + format_float_tostr(box_epa_test.penetration, 4) + "", 2000);
               //IM_Values.btn2 = false;
            }
            entityB->update();


            RENDER_MESSAGE(to_str(entityB->position), 1);

            vec3 startpoint = entityA->position + vec3(entityA->scale.x / 2.0, entityA->scale.y + 0.2, entityA->scale.z / 2.0);
            vec3 endpoint   = startpoint + box_epa_test.direction * entityA->scale.x / 2.0f;
            vec3 penetration_point = endpoint - box_epa_test.direction * box_epa_test.penetration;

            // IM_RENDER.add_line(IMHASH, startpoint, endpoint, 2, true, vec3(0.96,0.24,0.24));
            // IM_RENDER.add_point(IMHASH, startpoint, 3, true, vec3(0, 0.24, 0.96));
            // IM_RENDER.add_point(IMHASH, endpoint, 3, true, vec3(0, 0.96, 0.24));

            IM_RENDER.add_line(IMHASH, endpoint, penetration_point, 2, true, vec3(0.96,0.24,0.24));
         }
         else
         {
            RENDER_MESSAGE("No Penetration Found!", 2000);
         }

         //G_SCENE_INFO.tmp_unstuck_things = false;
      //}  
   }
   // RENDER_MESSAGE(box_gjk_test.collision ? "Collision!!" : "No Collision!" );
}