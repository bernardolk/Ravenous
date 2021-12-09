

RaycastTest CL_get_top_hit_from_multiple_raycasts(Ray first_ray, int qty, float spacing)
{
   /* Casts multiple ray towards the first_ray direction, with dir pointing upwards,
      qty says how many rays to shoot and spacing, well, the spacing between each ray. */

   float GRAB_REACH = G_SCENE_INFO.player->radius + 0.5;

   Ray ray = first_ray;
   float highest_y  = MIN_FLOAT;
   float shortest_z = MAX_FLOAT;
   RaycastTest best_hit;

   for_less(qty)
   {
      auto test = test_ray_against_scene(ray, RayCast_TestOnlyFromOutsideIn, G_SCENE_INFO.player->entity_ptr, GRAB_REACH);
      if(test.hit)
      {
         if(test.distance < shortest_z || (test.distance == shortest_z && highest_y < ray.origin.y))
         {
            highest_y = ray.origin.y;
            shortest_z = test.distance;
            best_hit = test;
         }
      }

      IM_RENDER.add_line(IM_ITERHASH(i), ray.origin, ray.origin + ray.direction * GRAB_REACH, 1.2, false, COLOR_GREEN_1);

      ray = Ray{ ray.origin + UNIT_Y * spacing, ray.direction };
   }

   if(best_hit.hit)
   {
      vec3 hitpoint = point_from_detection(best_hit.ray, best_hit);
      IM_RENDER.add_point(IMHASH, hitpoint, 2.0, true, COLOR_RED_1);
   }

   return best_hit;
}





