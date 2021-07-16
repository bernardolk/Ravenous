
bool p_anim_jumping_update(Player* player)
{
   // interpolate between 0 and duration the player's height
   float anim_d = P_ANIM_DURATION[P_ANIM_JUMPING];
   float new_height = P_HALF_HEIGHT - 0.1 * player->anim_t / anim_d;
   float h_diff = player->half_height - new_height;

   player->half_height = new_height;
   player->entity_ptr->collision_geometry.cylinder.half_length = new_height;
   player->entity_ptr->scale.y -= h_diff * 2;
   // compensates player shrinkage so he appears to be lifting the legs up
   player->entity_ptr->position.y += h_diff * 2;

   return false;
}


bool p_anim_landing_update(Player* player)
{
   bool interrupt = false;
   // add a linear height step of 0.3m per second
   float a_step = 0.3 * G_FRAME_INFO.duration; 
   float new_h = player->half_height + a_step;
   if(new_h >= P_HALF_HEIGHT)
   {
      new_h = P_HALF_HEIGHT;
      interrupt = true;
   }

   player->half_height = new_h;
   player->entity_ptr->collision_geometry.cylinder.half_length = new_h;
   player->entity_ptr->scale.y += a_step * 2;

   return interrupt;
}