void CL_wall_slide_player(Player* player, EPA_Result epa)
{
   // changes player velocity to be facing a wall parallel and dampens his speed
   auto& v = player->entity_ptr->velocity;
   if(v.x == 0 && v.z == 0)
      return;

   // UNCOMMENT THIS TO DAMPEN PLAYER SPEED WHEN COLLIDING
   // player->speed = player->speed > player->run_speed / 2.f ? player->run_speed / 2.f : player->speed;

   vec3 up_vec = vec3(0, 1, 0);
   vec3 right_vec = cross(up_vec, epa.direction);
   vec3 left_vec = -right_vec;

   if(dot(right_vec, v) > 0)
      v = right_vec * player->speed;
   else if(dot(left_vec, v) > 0)
      v = left_vec * player->speed;
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