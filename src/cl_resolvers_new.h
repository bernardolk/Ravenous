void CL_wall_slide_player(Player* player, vec3 wall_normal)
{
   // changes player velocity to be facing a wall parallel and dampens his speed
   auto& v = player->entity_ptr->velocity;
   if(v.x == 0 && v.z == 0)
      return;

   // UNCOMMENT THIS TO DAMPEN PLAYER SPEED WHEN COLLIDING
   // player->speed = player->speed > player->run_speed / 2.f ? player->run_speed / 2.f : player->speed;

   vec3 up_vec       = vec3(0, 1, 0);
   vec3 right_vec    = cross(up_vec, wall_normal);
   vec3 left_vec     = -right_vec;

   if(dot(right_vec, v) > 0)
   {
      auto horiz_v = right_vec * player->speed * dot(right_vec, v);
      v.x = horiz_v.x;
      v.z = horiz_v.z;
   }
   else if(dot(left_vec, v) > 0)
   {
      auto horiz_v = left_vec * player->speed * dot(left_vec, v);
      v.x = horiz_v.x;
      v.z = horiz_v.z;
   }
   else 
     player->brute_stop();

   // IM_RENDER.add_line(IMHASH, player->entity_ptr->position,
   //    player->entity_ptr->position +  v * 3, 
   //    2.0,
   //    true,
   //    vec3(0.9,0.24,0.24)
   // );

   // IM_RENDER.add_line(IMHASH, player->entity_ptr->position,
   //    player->entity_ptr->position +  v * 3, 
   //    2.0,
   //    true,
   //    vec3(0.9,0.24,0.24)
   // );
}

void CL_new_resolve_collision(CL_Results results, Player* player)
{
   CL_wall_slide_player(player, results.normal);

   switch(player->player_state)
   {
      case PLAYER_STATE_JUMPING:
         P_change_state(player, PLAYER_STATE_FALLING);
         break;
      case PLAYER_STATE_FALLING:
         // collided_with_floor 
         if(dot(results.normal, vec3(0,1,0)) > 0)
            P_state_change_falling_to_standing(player, results.entity);
         break;
   }
}