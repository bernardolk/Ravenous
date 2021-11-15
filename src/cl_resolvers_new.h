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

void CL_new_resolve_collision(CL_Results results, Player* player)
{
   bool collided_with_terrain = dot(results.normal, UNIT_Y) > 0.1;

   if(!collided_with_terrain)
      CL_wall_slide_player(player, results.normal);

   switch(player->player_state)
   {
      case PLAYER_STATE_JUMPING:
         P_change_state(player, PLAYER_STATE_FALLING);
         break;
      case PLAYER_STATE_FALLING:
         // collided_with_floor 
         if(collided_with_terrain)
         {
            // Add terrain to ignored collision list
            CL_Ignore_Colliders.add(results.entity);
            P_state_change_falling_to_standing(player, results.entity);
         }
         break;
   }
}