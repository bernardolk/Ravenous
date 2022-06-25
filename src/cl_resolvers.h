void CL_resolve_collision(CL_Results results, Player* player);
void CL_wall_slide_player(Player* player, vec3 wall_normal);
bool GP_simulate_player_collision_in_falling_trajectory(Player* player, vec2 xz_velocity);
bool CL_run_tests_for_fall_simulation(Player* player);

// fwd decl.
void GP_update_player_state(Player* &player);
CL_Results CL_test_player_vs_entity(Entity* entity, Player* player);

float PLAYER_STEPOVER_LIMIT   = 0.21;


// ---------------------
// > RESOLVE COLLISION
// ---------------------

void CL_resolve_collision(CL_Results results, Player* player)
{
   // unstuck player
   player->entity_ptr->position += results.normal * results.penetration;
   player->entity_ptr->update();
}

struct CL_VtraceResult {
   bool hit = false;
   float delta_y;
   Entity* entity;
};


CL_VtraceResult CL_do_stepover_vtrace(Player* player)
{
   /* Cast a ray at player's last point of contact with terrain to look for something steppable (terrain).
      Will cull out any results that are to be considered too high (is a wall) or too low (is a hole) considering
      player's current height. */

   vec3 ray_origin      = player->last_terrain_contact_point() + vec3(0, 0.21, 0);
   auto downward_ray    = Ray{ ray_origin, -UNIT_Y };
   RaycastTest raytest  = test_ray_against_scene(downward_ray, RayCast_TestOnlyFromOutsideIn, player->entity_ptr);

   if(!raytest.hit) 
      return CL_VtraceResult{ false };

   // auto angle = dot(get_triangle_normal(raytest.t), UNIT_Y);
   // std::cout << "Angle is: " << to_string(angle) << "\n";
   // if(angle < 1 - 0.866)
   //    return CL_VtraceResult{ false };

   
    // draw arrow
   auto hitpoint = point_from_detection(downward_ray, raytest);
   IM_RENDER.add_line(IMHASH, hitpoint, ray_origin, 1.0, true, COLOR_GREEN_1);
   IM_RENDER.add_point(IMHASH, hitpoint, 1.0, true, COLOR_GREEN_3);

   if(abs(player->entity_ptr->position.y - hitpoint.y) <= PLAYER_STEPOVER_LIMIT)
      return CL_VtraceResult{ true, player->last_terrain_contact_point().y - hitpoint.y, raytest.entity };

   return CL_VtraceResult{ false };
}


bool GP_simulate_player_collision_in_falling_trajectory(Player* player, vec2 xz_velocity)
{
   /*    Simulates how it would be if player fell following the xz_velocity vector.
         If player can get in a position where he is not stuck, we allow him to fall. 
   */

   // configs
   float d_frame = 0.014;

   auto pos_0     = player->entity_ptr->position;
   vec3 vel       = to3d_xz(xz_velocity);

   float max_iterations = 120;

   IM_RENDER.add_point(IMHASH, player->entity_ptr->position, 2.0, false, COLOR_GREEN_1, 1);

   int iteration = 0;
   while(true)
   {
      vel += d_frame * player->gravity; 
      player->entity_ptr->position += vel * d_frame;
      IM_RENDER.add_point(IM_ITERHASH(iteration), player->entity_ptr->position, 2.0, true, COLOR_GREEN_1, 1);

      player->entity_ptr->update();

      bool collided = CL_run_tests_for_fall_simulation(player);
      if(!collided)
         break;

      iteration++;
      if(iteration == max_iterations)
      {
         // if entered here, then we couldn't unstuck the player in max_iterations * d_frame seconds of falling towards
         // player movement direction, so he can't fall there
         player->entity_ptr->position = pos_0;
         player->entity_ptr->update();
         return false;
      }
   }

   player->entity_ptr->position = pos_0;
   player->entity_ptr->update();
   return true;
}

// ---------------------
// > WALL SLIDE PLAYER
// ---------------------
void CL_wall_slide_player(Player* player, vec3 wall_normal)
{
   // changes player velocity to be facing a wall parallel and dampens his speed
   auto& pv = player->entity_ptr->velocity;
   if(pv.x == 0 && pv.z == 0)
      return;

   // @todo - this is not good, need to figure out a better solution for
   //       speed when hitting walls
   float wall_slide_speed_limit = 1;
   if(player->speed > wall_slide_speed_limit)
      player->speed = wall_slide_speed_limit;

   vec3 up_vec       = vec3(0, 1, 0);
   vec3 horiz_vec    = cross(up_vec, wall_normal);

   pv = dot(pv, horiz_vec) * normalize(horiz_vec) * player->speed;
}


// --------------------------------------
// > CL_run_tests_for_fall_simulation
// --------------------------------------
bool CL_run_tests_for_fall_simulation(Player* player)
{ 
   /* It basically the usual test but without collision resolving. */

   auto entity_buffer = G_BUFFERS.entity_buffer;
   auto buffer = entity_buffer->buffer;
   auto entity_list_size = entity_buffer->size;
   bool terrain_collision = false;

   for (int i = 0; i < entity_list_size; i++)
   {
      Entity* &entity = buffer->entity;

      if(entity->name == "Player")
      {
         buffer++;
         continue;
      }

      // @TODO - here should test for bounding box collision (or any geometric first pass test) 
      //          FIRST, then do the call below

      auto result = CL_test_player_vs_entity(entity, player);

      if(result.collision)
      {
        return true;
      }
      
      buffer++;
   }

   return false;
}