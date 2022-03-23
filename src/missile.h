
float MISSILE_LIN_SPEED = 3.5;
float MISSILE_ANG_SPEED = 180;
float MIN_DODGE_DISTANCE = 1.5;
float MISSILE_INV_PERIOD = 2;

vec4 MISSILE_INITIAL_HEADING = vec4(0, 0, -1, 1);

bool UPDATE_MISSILE = false;

bool Launch = false;

void update_missile(Player* player, Entity* missile)
{
   /*    
      FIGURED IT OUT: We need a pure rotation matrix to extract euler angles otherwise we get 
      something fucked up. That is, we can't translate nor scale the matrix.
   */

   // compute rotation mat
   mat4 missile_rot  = rotate(mat4identity, glm::radians(missile->rotation.x), vec3(1.0f, 0.0f, 0.0f));
   missile_rot       = rotate(missile_rot,  glm::radians(missile->rotation.y), vec3(0.0f, 1.0f, 0.0f));
   missile_rot       = rotate(missile_rot,  glm::radians(missile->rotation.z), vec3(0.0f, 0.0f, 1.0f));

   // compute heading (H), target (D) and rotation axis (R)
   vec3 H      = to_vec3(missile_rot * MISSILE_INITIAL_HEADING);
   vec3 dist   = player->eye() - vec3(0, 0.2, 0) - missile->position;
   vec3 D      = normalize(dist);
   vec3 R      = normalize(cross(D, H));

   // update position
   missile->position += normalize(H) * MISSILE_LIN_SPEED * G_FRAME_INFO.duration;

   if(!missile->dodged)
   {
       // check for dodge
      if(player->dodge_btn && length(dist) < MIN_DODGE_DISTANCE)
      {
         missile->dodged = true;
         GP_change_player_state(player, PLAYER_STATE_JUMPING);
      }
      else
      {
         // apply rotation along rotation axis R
         float ang_speed  = -MISSILE_ANG_SPEED * G_FRAME_INFO.duration;
         missile_rot = rotate(mat4identity, glm::radians(ang_speed), R) * missile_rot;

         // extract euler angles from new rot matrix using XYZ convention
         float yaw, pitch, roll;
         extractEulerAngleXYZ(missile_rot, pitch, yaw, roll);

         yaw      = glm::degrees(yaw);
         pitch    = glm::degrees(pitch);
         roll     = glm::degrees(roll); 
         
         // update missile orientation euler angles
         missile->rotation.x = pitch;
         missile->rotation.y = yaw;
         missile->rotation.z = roll;
      }
   }
   else
   {
      missile->inv_period_timer += G_FRAME_INFO.duration;

      editor_print("Player is invincible against missile right now!", 2000, COLOR_GREEN_2);

      if(missile->inv_period_timer >= MISSILE_INV_PERIOD)
      {
         missile->dodged = false;
         missile->inv_period_timer = 0;
      }
   }

   // update missile
   missile->update();

   if(length(dist) < MIN_DODGE_DISTANCE)
   {
      IM_RENDER.add_line(
         IMHASH, missile->position + vec3(missile->scale.x / 2.0f, missile->scale.y / 2.0f, 0), 
         missile->position + dist, 2.0f, false, COLOR_GREEN_1
      );
   }
   else
   {
      IM_RENDER.add_line(
         IMHASH, missile->position + vec3(missile->scale.x / 2.0f, missile->scale.y / 2.0f, 0), 
         missile->position + dist, 1.2f, false, COLOR_RED_1
      );
   }


   // render vectors
   // IM_RENDER.add_line(IMHASH, missile->position, missile->position + H * 2.f, 1.2f, false, COLOR_RED_1);
   IM_RENDER.add_line(IMHASH, missile->position, missile->position + R * 2.f, 1.2f, false, COLOR_YELLOW_1);
}