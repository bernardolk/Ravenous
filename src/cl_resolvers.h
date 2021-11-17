
void CL_resolve_collision(CL_Results results, Player* player);
void CL_wall_slide_player(Player* player, vec3 wall_normal);

// fwd decl.
void GP_update_player_state(Player* &player);

// ---------------------
// > RESOLVE COLLISION
// ---------------------
// @NOTE: I am pretty much not convinced that this is the right way to handle post-collision player state changes
//    in terms of project module divisions. This code should be somewhere in the GP update module.

void CL_resolve_collision(CL_Results results, Player* player)
{
   bool collided_with_terrain = dot(results.normal, UNIT_Y) > 0.1;

   if(!collided_with_terrain)
   {
      // unstuck player
      player->entity_ptr->position += results.normal * results.penetration;
      player->entity_ptr->update();

      /*NOTE: if we unstuck the player when colliding with terrain, in the case that the player
            would fall from edge, we would prevent next frame collisions with the terrain collider
            and then it would be removed from CL_IgnoredColliders list, thus, looping from falling
            to standing to falling ... */

      CL_wall_slide_player(player, results.normal);
   }

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
            // // Add terrain to ignored collision list
            // CL_Ignore_Colliders.add(results.entity);
            // auto args = PlayerStateChangeArgs();
            // args.entity = results.entity;
               
            // // check if player stepped correctly into floor (his center lies on top of floor)
            // auto c_vtrace = CL_do_c_vtrace(player);
            // if(c_vtrace.hit)
            // {
            //    GP_change_player_state(player, PLAYER_STATE_STANDING, args);
            // }
            // else
            // {

            //    bool can_fall = GP_simulate_player_collision_in_falling_trajectory(player);
            //    if(can_fall)
            //    {
            //       // @TODO - This here is not working properly yet
                  
            //        // final player position after falling from the edge (when he stops touching anything)
            //       vec3 terminal_position = player->entity_ptr->position;
            //       //IM_RENDER.add_mesh(IMHASH, &player->entity_ptr->collider);

            //       GP_change_player_state(player, PLAYER_STATE_FALLING);
            //    }
            //    else
            //    {
            //       // @todo - still undefined
            //       GP_change_player_state(player, PLAYER_STATE_STANDING, args);
            //    }
            // }
            
            GP_change_player_state(player, PLAYER_STATE_STANDING);
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

   // @todo - this is not good, need to figure out a better solution for
   //       speed when hitting walls
   float wall_slide_speed_limit = 1;
   if(player->speed > wall_slide_speed_limit)
      player->speed = wall_slide_speed_limit;

   vec3 up_vec       = vec3(0, 1, 0);
   vec3 horiz_vec    = cross(up_vec, wall_normal);

   pv = dot(pv, horiz_vec) * normalize(horiz_vec) * player->speed;
}