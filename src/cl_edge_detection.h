

RaycastTest CL_get_top_hit_from_multiple_raycasts(Ray first_ray, int qty, float spacing, Player* player)
{
   /* Casts multiple ray towards the first_ray direction, with dir pointing upwards,
      qty says how many rays to shoot and spacing, well, the spacing between each ray. */

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


void CL_perform_edge_detection(Player* player)
{
   auto orientation_xz = to_xz(player->orientation);
   auto first_ray = Ray{player->eye() - UNIT_Y * 0.2f, orientation_xz};
   const float _front_ray_spacing   = 0.03;
   const float _front_ray_qty       = 10;

   auto front_test = CL_get_top_hit_from_multiple_raycasts(first_ray, _front_ray_qty, _front_ray_spacing, player);
   if(front_test.hit)
   {
      vec3 frontal_hitpoint = point_from_detection(front_test.ray, front_test);

      const float _top_ray_height = 2.0f;
      auto top_ray = Ray{frontal_hitpoint + front_test.ray.direction * 0.0001f + UNIT_Y * _top_ray_height, -UNIT_Y};

      auto top_test = test_ray_against_scene(
         top_ray, RayCast_TestOnlyFromOutsideIn, G_SCENE_INFO.player->entity_ptr, _top_ray_height
      );

      if(top_test.hit)
      {
         vec3 top_hitpoint = point_from_detection(top_test.ray, top_test);

         if(top_test.distance <= player->height || top_hitpoint.y - frontal_hitpoint.y > _front_ray_spacing)
            return;

         IM_RENDER.add_line(IMHASH, top_ray.origin, frontal_hitpoint, 1.2, false, COLOR_PURPLE_1);
         IM_RENDER.add_point(IMHASH, top_hitpoint, 2.0, true, COLOR_PURPLE_1);
      }

   }

}




