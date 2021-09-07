void AN_animate_player              (Player* player); 
void AN_p_anim_force_interrupt      (Player* player); 
bool AN_p_anim_jumping_update       (Player* player); 
bool AN_p_anim_landing_update       (Player* player); 
bool AN_p_anim_landing_fall_update  (Player* player); 
bool AN_p_anim_vaulting             (Player* player); 


void AN_animate_player(Player* player)
{
   auto& anim_s         = player->anim_state;
   auto ANIM_DURATION   = P_ANIM_DURATION[anim_s];
   if(anim_s == P_ANIM_NO_ANIM)
      return;

   auto& anim_t         = player->anim_t;
   anim_t               += G_FRAME_INFO.duration * 1000;

   bool end_anim = false;
   if(ANIM_DURATION > 0 && anim_t >= ANIM_DURATION)
   {
      anim_t            = ANIM_DURATION;
      end_anim          = true;
   }

   bool interrupt       = false;
   switch(anim_s)
   {
      case P_ANIM_JUMPING:
         interrupt                  = AN_p_anim_jumping_update(player);
         break;
      case P_ANIM_LANDING:
         interrupt                  = AN_p_anim_landing_update(player);
         break;
      case P_ANIM_LANDING_FALL:
         interrupt                  = AN_p_anim_landing_fall_update(player);
         break;
      case P_ANIM_VAULTING:
         interrupt                  = AN_p_anim_vaulting(player);
         if(interrupt)
            GP_finish_vaulting(player);
         break;
   }

   if(end_anim || interrupt)
   {
      anim_s = P_ANIM_NO_ANIM;
      anim_t = 0;
   }
}


bool AN_p_anim_jumping_update(Player* player)
{
   // interpolate between 0 and duration the player's height
   float anim_d            = P_ANIM_DURATION[P_ANIM_JUMPING];
   float new_half_height   = P_HALF_HEIGHT - 0.1 * player->anim_t / anim_d;
   float h_diff            = player->half_height - new_half_height;

   player->half_height = new_half_height;
   player->entity_ptr->collision_geometry.cylinder.half_length = new_half_height;
   player->entity_ptr->scale.y -= h_diff;
   // compensates player shrinkage so he appears to be lifting the legs up
   player->entity_ptr->position.y += h_diff * 2;

   return false;
}


bool AN_p_anim_landing_update(Player* player)
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


bool AN_p_anim_landing_fall_update(Player* player)
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


bool AN_p_anim_vaulting(Player* player)
{
   G_INPUT_INFO.block_mouse_move = true;

   vec3& p_pos = player->entity_ptr->position;

   // animation speed in m/s
   const float v_y  = 2.f / 1.f;
   const float v_xz = 2.f / 2.f;

   vec3 d      = player->anim_final_pos - p_pos;
   vec3 d_orig = player->anim_final_pos - player->anim_orig_pos;

   float d_orig_y_sig = sign(d_orig.y);
   float d_orig_x_sig = sign(d_orig.x);
   float d_orig_z_sig = sign(d_orig.z);

   float y_dist = abs(d.y);
   float x_dist = abs(d.x);
   float z_dist = abs(d.z);

   float ds_y = v_y  * G_FRAME_INFO.duration;
   float ds_x = v_xz * G_FRAME_INFO.duration;
   float ds_z = v_xz * G_FRAME_INFO.duration;

   float ds_y_sig = sign(d.y);
   float ds_x_sig = sign(d.x);
   float ds_z_sig = sign(d.z);

   if(y_dist >= ds_y && d_orig_y_sig == ds_y_sig)
      p_pos.y += ds_y_sig * ds_y;
   else
      p_pos.y = player->anim_final_pos.y;
   
   if(x_dist >= ds_x && d_orig_x_sig == ds_x_sig)
      p_pos.x += ds_x_sig * ds_x;
   else
      p_pos.x = player->anim_final_pos.x;

   if(z_dist >= ds_z && d_orig_z_sig == ds_z_sig)
      p_pos.z += ds_z_sig * ds_z;
   else
      p_pos.z = player->anim_final_pos.z;

   // camera direction animation
   if(!player->anim_finished_turning)
   {
      vec2 f_dir_xz        = to2d_xz(player->anim_final_dir);
      float orig_sva       = vector_angle_signed(nrmlz(to2d_xz(player->anim_orig_dir)), f_dir_xz);
      float orig_angle     = glm::degrees(orig_sva);
      float orig_sign      = sign(orig_angle);
      float turn_angle     = 0.5 * orig_sign;
      camera_change_direction(pCam, turn_angle, 0.f);

      float updated_sva    = vector_angle_signed(nrmlz(to2d_xz(pCam->Front)), f_dir_xz);
      float updated_angle  = glm::degrees(updated_sva);
      float updated_sign   = sign(updated_angle);
      if(updated_sign != orig_sign)
      {
         camera_change_direction(pCam, -1.0* updated_angle, 0.f);
         player->anim_finished_turning = true;
      }
   }

   /*
   RENDER_MESSAGE("front: " + to_string(nrmlz(to2d_xz(pCam->Front))));
   RENDER_MESSAGE("final dir: " + to_string(player->anim_final_dir), 0, vec3(0.8, 0.8, 0.8));
   RENDER_MESSAGE("orig angle: " + to_string(orig_angle), 0, vec3(0.8, 0.8, 0.8));
   RENDER_MESSAGE("current angle: " +  to_string(updated_angle));
   RENDER_MESSAGE("sva cam-final: " +  to_string(updated_sva), 0, vec3(0,0.8,0.1));
   RENDER_MESSAGE("sva orig-final: " +  to_string(orig_sva), 0, vec3(0,0.8,0.1));
   RENDER_MESSAGE("orig sign: " +  to_string(orig_sign), 0, vec3(0.8,0.0,0.1));
   RENDER_MESSAGE("updated sign: " +  to_string(updated_sign), 0, vec3(0.8,0.0,0.1));
   */

   if(is_equal(p_pos, player->anim_final_pos) && player->anim_finished_turning)
      return true;
   else
      return false;
}


void AN_p_anim_force_interrupt(Player* player)
{
   player->anim_state = P_ANIM_NO_ANIM;
   player->anim_t = 0;
   player->half_height = P_HALF_HEIGHT;
   player->entity_ptr->scale.y = P_HALF_HEIGHT;
   player->entity_ptr->collision_geometry.cylinder.half_length = P_HALF_HEIGHT;
}