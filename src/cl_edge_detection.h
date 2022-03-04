
#include <ledge.h>

RaycastTest CL_get_top_hit_from_multiple_raycasts(Ray first_ray, int qty, float spacing, Player* player)
{
   /* 
      Casts multiple ray towards the first_ray direction, with dir pointing upwards,
      qty says how many rays to shoot and spacing, well, the spacing between each ray.
   */

   Ray ray = first_ray;
   float highest_y  = MIN_FLOAT;
   float shortest_z = MAX_FLOAT;
   RaycastTest best_hit_results;

   for_less(qty)
   {
      auto test = test_ray_against_scene(ray, RayCast_TestOnlyFromOutsideIn, player->entity_ptr, player->grab_reach);
      if(test.hit)
      {
         if(test.distance < shortest_z || (are_equal_floats(test.distance, shortest_z) && highest_y < ray.origin.y))
         {
            highest_y         = ray.origin.y;
            shortest_z        = test.distance;
            best_hit_results  = test;
         }
      }

      IM_RENDER.add_line(IM_ITERHASH(i), ray.origin, ray.origin + ray.direction * player->grab_reach, 1.2, false, COLOR_GREEN_1);

      ray = Ray{ ray.origin + UNIT_Y * spacing, ray.direction };
   }

   if(best_hit_results.hit)
   {
      vec3 hitpoint = point_from_detection(best_hit_results.ray, best_hit_results);
      IM_RENDER.add_point(IMHASH, hitpoint, 2.0, true, COLOR_RED_1);
   }

   return best_hit_results;
}


Ledge CL_perform_ledge_detection(Player* player)
{
   // concepts: front face - where the horizontal rays are going to hit
   //           top face - where the vertical ray (up towards down) is going to hit
   Ledge ledge;

   // settings
   const float _front_ray_first_ray_delta_y     = 0.6;
   const float _front_ray_spacing               = 0.03;
   const float _front_ray_qty                   = 24;

   auto orientation_xz = to_xz(player->orientation);
   auto first_ray = Ray{player->eye() - UNIT_Y * _front_ray_first_ray_delta_y, orientation_xz};
   ledge.detection_direction = first_ray.direction;



   auto front_test = CL_get_top_hit_from_multiple_raycasts(first_ray, _front_ray_qty, _front_ray_spacing, player);
   if(front_test.hit)
   {
      vec3 frontal_hitpoint = point_from_detection(front_test.ray, front_test);
      vec3 front_face_n = get_triangle_normal(front_test.t);

      // checks if face "points downwards" like this: / (p)  and not like this \ (p) where (p) is player trying to grab ledge
      editor_print(to_string(dot(UNIT_Y, front_face_n)));

      if(dot(UNIT_Y, front_face_n) > 0.0001)
         return ledge;

      const float _top_ray_height = 2.0f;
      auto top_ray = Ray{frontal_hitpoint + front_test.ray.direction * 0.0001f + UNIT_Y * _top_ray_height, -UNIT_Y};

      auto top_test = test_ray_against_scene(
         top_ray, RayCast_TestOnlyFromOutsideIn, G_SCENE_INFO.player->entity_ptr, _top_ray_height
      );

      if(top_test.hit)
      {
         vec3 top_hitpoint    = point_from_detection(top_test.ray, top_test);
         ledge.surface_point  = top_hitpoint;

         if(top_test.distance <= player->height || top_hitpoint.y - frontal_hitpoint.y > _front_ray_spacing)
            return ledge;

         IM_RENDER.add_line(IMHASH, top_ray.origin, frontal_hitpoint, 1.2, false, COLOR_PURPLE_1);
         IM_RENDER.add_point(IMHASH, top_hitpoint, 2.0, true, COLOR_PURPLE_1);

         // test edges
         vec3 edge1 = top_test.t.b - top_test.t.a; // 1
         vec3 edge2 = top_test.t.c - top_test.t.b; // 2
         vec3 edge3 = top_test.t.a - top_test.t.c; // 3

         // for debug: show face normal
         vec3 front_face_center = get_barycenter(front_test.t);
         IM_RENDER.add_line(IMHASH, front_face_center, front_face_center + 1.f * front_face_n, 2.0, false, COLOR_BLUE_1);

         if(abs(dot(edge1, front_face_n)) < 0.0001)
         {
            IM_RENDER.add_line(IMHASH, top_test.t.a, top_test.t.a + edge1, 2.0, true, COLOR_YELLOW_1);
            IM_RENDER.add_point(IMHASH, top_test.t.a, 2.0, false, COLOR_YELLOW_1);
            IM_RENDER.add_point(IMHASH, top_test.t.b, 2.0, false, COLOR_YELLOW_1);

            ledge.a = top_test.t.a;
            ledge.b = top_test.t.b;

            ledge.empty = false;
            return ledge;
         }
         else if(abs(dot(edge2, front_face_n)) < 0.0001)
         {
            IM_RENDER.add_line(IMHASH, top_test.t.b, top_test.t.b + edge2, 2.0, true, COLOR_YELLOW_1);
            IM_RENDER.add_point(IMHASH, top_test.t.b, 2.0, false, COLOR_YELLOW_1);
            IM_RENDER.add_point(IMHASH, top_test.t.c, 2.0, false, COLOR_YELLOW_1);

            ledge.a = top_test.t.b;
            ledge.b = top_test.t.c;

            ledge.empty = false;
            return ledge;
         }
         else if(abs(dot(edge3, front_face_n)) < 0.0001)
         {
            IM_RENDER.add_line(IMHASH, top_test.t.c, top_test.t.c + edge3, 2.0, true, COLOR_YELLOW_1);
            IM_RENDER.add_point(IMHASH, top_test.t.c, 2.0, false, COLOR_YELLOW_1);
            IM_RENDER.add_point(IMHASH, top_test.t.a, 2.0, false, COLOR_YELLOW_1);

            ledge.a = top_test.t.c;
            ledge.b = top_test.t.a;

            ledge.empty = false;
            return ledge;
         }
         else
            editor_print("No ledge found! Strange isn't it?");
      }
   }

   ledge.empty = true;
   return ledge;
}

vec3 CL_get_final_position_ledge_vaulting(Player* player, Ledge ledge)
{
   /* Returns the player's position after finishing vaulting across the given ledge */
   vec3 inward_normal = normalize(cross(ledge.a - ledge.b, UNIT_Y));
   return ledge.surface_point + inward_normal * player->radius * 2.f;
}





