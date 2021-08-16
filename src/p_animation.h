
bool p_anim_jumping_update(Player* player)
{
   // interpolate between 0 and duration the player's height
   float anim_d = P_ANIM_DURATION[P_ANIM_JUMPING];
   float new_half_height = P_HALF_HEIGHT - 0.1 * player->anim_t / anim_d;
   float h_diff = player->half_height - new_half_height;

   player->half_height = new_half_height;
   player->entity_ptr->collision_geometry.cylinder.half_length = new_half_height;
   player->entity_ptr->scale.y -= h_diff;
   // compensates player shrinkage so he appears to be lifting the legs up
   player->entity_ptr->position.y += h_diff * 2;

   return false;
}


bool p_anim_landing_update(Player* player)
{
   bool interrupt = false;
   // add a linear height step of 0.5m per second
   float a_step = 0.5 * G_FRAME_INFO.duration; 
   float new_half_height = player->half_height + a_step;
   if(new_half_height >= P_HALF_HEIGHT)
   {
      new_half_height = P_HALF_HEIGHT;
      a_step = P_HALF_HEIGHT - player->entity_ptr->scale.y;
      interrupt = true;
   }

   player->half_height = new_half_height;
   player->entity_ptr->collision_geometry.cylinder.half_length = new_half_height;
   player->entity_ptr->scale.y += a_step;

   return interrupt;
}

bool p_anim_landing_fall_update(Player* player)
{
   float anim_d = P_ANIM_DURATION[P_ANIM_LANDING_FALL];
   bool interrupt = false;
   // sets the % of the duration of the animation that consists
   // of player bending his knees on the fall, the rest is standing up again
   float landing_d = anim_d * 0.25;

   // landing part
   if(player->anim_t <= landing_d)
   {
      float new_half_height = P_HALF_HEIGHT - 0.05 * player->anim_t / landing_d;
      float h_diff = player->half_height - new_half_height;

      player->half_height = new_half_height;
      player->entity_ptr->collision_geometry.cylinder.half_length = new_half_height;
      player->entity_ptr->scale.y -= h_diff;
   }
   // standing part
   else if(player->anim_t > landing_d)
   {
      float a_step = 0.5 * G_FRAME_INFO.duration; 
      float new_half_height = player->half_height + a_step;
      if(new_half_height >= P_HALF_HEIGHT)
      {
         new_half_height = P_HALF_HEIGHT;
         a_step = P_HALF_HEIGHT - player->entity_ptr->scale.y;
         interrupt = true;
      }

      player->half_height = new_half_height;
      player->entity_ptr->collision_geometry.cylinder.half_length = new_half_height;
      player->entity_ptr->scale.y += a_step;
   }

   return interrupt;
}

bool p_anim_vaulting(Player* player)
{
   // animation speed in m/s
   const float v_y = 1.f / 1.f;
   const float v_xz = 1.f / 2.f;

   vec3& p_pos = player->entity_ptr->position;

   if(is_equal(p_pos, player->desired_position))
      return true;
   
   vec3 d = player->desired_position - p_pos;
   vec3 dunit = glm::normalize(d);

   // adds to player's position a bit of movement towards destination
   // player always goes up (y) but might go anywhere in x-z, thus, squaring
   if(d.y > 0)
   {
      float ds = v_y * G_FRAME_INFO.duration;
      if(ds * ds < d.y * d.y)
         p_pos.y += dunit.y * ds;
      else 
         p_pos.y = player->desired_position.y;
   }
   if(d.x * d.x > 0)
   {
      float ds = v_xz * G_FRAME_INFO.duration;
      if(ds * ds < d.x * d.x)
         p_pos.x += dunit.x * ds;
      else 
         p_pos.x = player->desired_position.x;
   }
   if(d.z * d.z > 0)
   {
      float ds = v_xz * G_FRAME_INFO.duration;
      if(ds * ds < d.z * d.z)
         p_pos.z += dunit.z * ds;
      else 
         p_pos.z = player->desired_position.z;
   }

   return false;
}


void p_anim_force_interrupt(Player* player)
{
   player->anim_state = P_ANIM_NO_ANIM;
   player->anim_t = 0;
   player->half_height = P_HALF_HEIGHT;
   player->entity_ptr->scale.y = P_HALF_HEIGHT;
   player->entity_ptr->collision_geometry.cylinder.half_length = P_HALF_HEIGHT;
}