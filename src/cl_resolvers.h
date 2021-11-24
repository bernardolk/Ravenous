
void CL_resolve_collision(CL_Results results, Player* player);
void CL_wall_slide_player(Player* player, vec3 wall_normal);
bool GP_simulate_player_collision_in_falling_trajectory(Player* player, vec2 xz_velocity);
RaycastTest CL_do_c_vtrace(Player* player);
bool CL_run_tests_for_fall_simulation(Player* player);

// fwd decl.
void GP_update_player_state(Player* &player);
CL_Results CL_test_player_vs_entity(Entity* entity, Player* player);

float PLAYER_STEPOVER_LIMIT   = 0.21;


// ---------------------
// > RESOLVE COLLISION
// ---------------------
// @NOTE: I am pretty much not convinced that this is the right way to handle post-collision player state changes
//    in terms of project module divisions. This code should be somewhere in the GP update module.

void CL_resolve_collision(CL_Results results, Player* player)
{
   bool collided_with_terrain = dot(results.normal, UNIT_Y) > 0;

   // if(collided_with_terrain)
   // {
   //    // Add terrain to ignored collision list
   //    CL_Ignore_Colliders.add(results.entity);
   //    auto args = PlayerStateChangeArgs();
   //    args.entity = results.entity;
   // }

   // unstuck player
   player->entity_ptr->position += results.normal * results.penetration;
   player->entity_ptr->update();

   if(!collided_with_terrain)
      CL_wall_slide_player(player, results.normal);

   switch(player->player_state)
   {
      case PLAYER_STATE_JUMPING:
         // @todo - need to include case that player touches inclined terrain
         //          in that case it should also stand (or fall from edge) and not
         //          directly fall.
         GP_change_player_state(player, PLAYER_STATE_FALLING);
         break;
      case PLAYER_STATE_FALLING:
         // collided_with_floor 
         if(collided_with_terrain)
            GP_change_player_state(player, PLAYER_STATE_STANDING);
         
         break;
      case PLAYER_STATE_STANDING:
      {
         // DO PLAYER CENTER VTRACE IN NEXT POSITION
         auto c_vtrace = CL_do_c_vtrace(player);

         if(!c_vtrace.hit)
         {
            /* If player was standing on something and we added the collider to the ignored colliders for collision,
               then, run a simulation, moving player like gravity would, testing for collision in each step and
               once we are not colliding with the terrain colliders anymore, check if player fits without colliding with
               anything in that position and then allow him to fall through.
               If he doesn't fit, then ignore the hole and let player walk through it.
            */

            // give player a push if necessary
            float fall_momentum_intensity = player->speed;
            if(fall_momentum_intensity < player->fall_from_edge_push_speed)
               fall_momentum_intensity = player->fall_from_edge_push_speed;


            vec2 fall_momentum_dir;
            fall_momentum_dir = to2d_xz(player->v_dir_historic);
            vec2 fall_momentum = fall_momentum_dir * fall_momentum_intensity;

            bool can_fall = GP_simulate_player_collision_in_falling_trajectory(player, fall_momentum);

            if(can_fall)
            {
               player->entity_ptr->velocity = to3d_xz(fall_momentum);

               GP_change_player_state(player, PLAYER_STATE_FALLING);
            }
            else
            {
               RENDER_MESSAGE("Player won't fit if he falls here.", 1000);
            }
         }
         
         player->update();
      }
   }
}

//@todo - Rethink the name and purpose of this function
RaycastTest CL_do_c_vtrace(Player* player)
{
   // stands for Central Vertical Trace, basically, look below player's center for something steppable (terrain)

   auto downward_ray    = Ray{player->feet() + vec3{0.0f, PLAYER_STEPOVER_LIMIT, 0.0f}, -UNIT_Y};
   RaycastTest raytest  = test_ray_against_scene(downward_ray, RayCast_TestOnlyFromOutsideIn, player->entity_ptr);

   if(!raytest.hit) return RaycastTest{false};

    // draw arrow
   auto hitpoint = point_from_detection(downward_ray, raytest);
   IM_RENDER.add_line(IMHASH, hitpoint, hitpoint + UNIT_Y * 3.f, 1.0, true, COLOR_GREEN_1);
   IM_RENDER.add_point(IMHASH, hitpoint, 1.0, true, COLOR_GREEN_3);

   if(abs(raytest.distance - PLAYER_STEPOVER_LIMIT) <= PLAYER_STEPOVER_LIMIT)
      return RaycastTest{true, raytest.distance - PLAYER_STEPOVER_LIMIT, raytest.entity};
   else
      return RaycastTest{false};
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
   /* This will test collision against entities in collision buffer and
      check whether the collided entity is present in the CL_IgnoredColliders list.
      We will keep testing until we have no collisions or we collide with an entity
      that is NOT in the list. This way we know the play can either fall or not.
      (he was able to leave the terrain and either be free or collided with something
      else before being free, leaving him stuck inside the terrain)
   */

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