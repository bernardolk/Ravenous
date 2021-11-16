
void CL_resolve_collision(CL_Results results, Player* player);
void CL_wall_slide_player(Player* player, vec3 wall_normal);

// fwd decl.
RaycastTest CL_do_c_vtrace                      (Player* player);
bool GP_simulate_player_collision_in_falling_trajectory(Player* player);

// ---------------------
// > RESOLVE COLLISION
// ---------------------
// @NOTE: I am pretty much not convinced that this is the right way to handle post-collision player state changes
//    in terms of project module divisions. This code should be somewhere in the GP update module.

void CL_resolve_collision(CL_Results results, Player* player)
{
   bool collided_with_terrain = dot(results.normal, UNIT_Y) > 0.1;

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
         {
            // Add terrain to ignored collision list
            CL_Ignore_Colliders.add(results.entity);
            auto args = PlayerStateChangeArgs();
            args.entity = results.entity;
               
            // check if player stepped correctly into floor (his center lies on top of floor)
            auto c_vtrace = CL_do_c_vtrace(player);
            if(c_vtrace.hit)
            {
               GP_change_player_state(player, PLAYER_STATE_STANDING, args);
            }
            else
            {
               bool can_fall = GP_simulate_player_collision_in_falling_trajectory(player);
               if(can_fall)
               {
                  // @TODO - This here is not working properly yet
                  
                   // final player position after falling from the edge (when he stops touching anything)
                  vec3 terminal_position = player->entity_ptr->position;
                  //IM_RENDER.add_mesh(IMHASH, &player->entity_ptr->collider);

                  auto& vel = player->entity_ptr->velocity;
                  if(abs(vel.x) < player->fall_from_edge_push_speed && abs(vel.z) < player->fall_from_edge_push_speed)
                     vel = -player->v_dir_historic * player->fall_from_edge_push_speed;

                  GP_change_player_state(player, PLAYER_STATE_FALLING);
               }
               else
               {
                  // @todo - still undefined
                  GP_change_player_state(player, PLAYER_STATE_STANDING, args);
               }
            }

            
         }
         break;
   }
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

   // UNCOMMENT THIS TO DAMPEN PLAYER SPEED WHEN COLLIDING
   // player->speed = player->speed > player->run_speed / 2.f ? player->run_speed / 2.f : player->speed;

   vec3 up_vec       = vec3(0, 1, 0);
   vec3 horiz_vec    = cross(up_vec, wall_normal);

   pv = dot(pv, horiz_vec) * normalize(horiz_vec) * player->speed;
}