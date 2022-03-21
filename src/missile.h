

struct Heading {
   float pitch;
   float yaw;
   vec3 dir;
};

Heading look_at(Entity* entity, vec3 position);

// vec3 Missile_Heading = vec3(0, 0, -1);

float MISSILE_LIN_SPEED = 1;
float MISSILE_ANG_SPEED = 30;

vec4 MISSILE_INITIAL_POS = vec4(0, 0, -1, 1);

bool UPDATE_MISSILE = false;

// mat4 Missile_rotation_mat()
// {
//    mat4 rot_heading      = rotate(mat4identity, glm::radians(0.f), vec3(1.0f, 0.0f, 0.0f));
//    rot_heading           = rotate(rot_heading, glm::radians(45.f), vec3(0.0f, 1.0f, 0.0f));
//    rot_heading           = rotate(rot_heading, glm::radians(0.f), vec3(0.0f, 0.0f, 1.0f));

//    return rot_heading;
// }

// mat4 Missile_Rot = Missile_rotation_mat();

float Ang = 0;

void update_missile(Player* player, Entity* missile)
{

   /* 
      Okay so this is quite hard for me ...
      We update the model position based on extrinsic euler angles each frame.
      I can get and rotate a heading vector, but I can't get back the extrinsic euler angles FROM that
      rotated vector.
      I believe this is because _I don't really know_ which euler angle conventions I am using.
      I could either extract the angles from the rot matrix using glm once I figure out, or,
      compute the angles from the direction vector.
      But then I would need to make sure to compute extrinsic angles and not intrinsic (based on rot_axis),
      otherwise I would need to change the basis back to world axis.
   */
   
   mat4 missile_rot  = rotate(mat4identity, glm::radians(missile->rotation.x), vec3(1.0f, 0.0f, 0.0f));
   missile_rot       = rotate(missile_rot,  glm::radians(missile->rotation.y), vec3(0.0f, 1.0f, 0.0f));
   missile_rot       = rotate(missile_rot,  glm::radians(missile->rotation.z), vec3(0.0f, 0.0f, 1.0f));

   vec3 H = to_vec3(missile_rot * MISSILE_INITIAL_POS);
   vec3 D = normalize(player->entity_ptr->position - missile->position);
   vec3 R = cross(D, H);

   float ang_speed = -MISSILE_ANG_SPEED * G_FRAME_INFO.duration;
   missile_rot = rotate(mat4identity, glm::radians(ang_speed), R) * missile_rot;
   H = to_vec3(missile_rot * MISSILE_INITIAL_POS);
   
   float yaw, pitch, roll;
   extractEulerAngleYZX(missile_rot, yaw, pitch, roll);

   editor_print(to_string(vec3(glm::degrees(yaw), glm::degrees(pitch), glm::degrees(roll))));

   // if (UPDATE_MISSILE)
   // {
      // Ang += 10;
   // }

   missile->rotation.x = glm::degrees(pitch);
   missile->rotation.y = glm::degrees(yaw);

   // log(LOG_INFO, "rotation:" + to_string(vec2(missile->rotation.x, missile->rotation.y)));

   IM_RENDER.add_line(IMHASH, missile->position, missile->position + D * 2.f, 1.2f, false, COLOR_GREEN_1);
   IM_RENDER.add_line(IMHASH, missile->position, missile->position + H * 2.f, 1.2f, false, COLOR_RED_1);
   IM_RENDER.add_line(IMHASH, missile->position, missile->position + R * 2.f, 1.2f, false, COLOR_YELLOW_1);




}

void update_missile2(Player* player, Entity* missile)
{

   /* 
      Okay so this is quite hard for me ...
      We update the model position based on extrinsic euler angles each frame.
      I can get and rotate a heading vector, but I can't get back the extrinsic euler angles FROM that
      rotated vector.
      I believe this is because _I don't really know_ which euler angle conventions I am using.
      I could either extract the angles from the rot matrix using glm once I figure out, or,
      compute the angles from the direction vector.
      But then I would need to make sure to compute extrinsic angles and not intrinsic (based on rot_axis),
      otherwise I would need to change the basis back to world axis.
   */
   

   if (UPDATE_MISSILE)
   {
      Heading new_heading = look_at(missile, player->entity_ptr->position);

      log(LOG_INFO, "new_heading:" + to_string(vec2(new_heading.pitch, new_heading.yaw)));

      int yaw_dir = 1, pitch_dir = 1;
      if (new_heading.yaw < missile->rotation.y)    yaw_dir   *= -1;
      if (new_heading.pitch < missile->rotation.x)  pitch_dir *= -1;

      float delta_angle_yaw   = MISSILE_ANG_SPEED * G_FRAME_INFO.duration;
      float delta_angle_pitch = delta_angle_yaw;

      float distance_yaw = abs(new_heading.yaw - missile->rotation.y);
      if (delta_angle_yaw > distance_yaw) 
         delta_angle_yaw = new_heading.yaw - missile->rotation.y;

      float distance_pitch = abs(new_heading.pitch - missile->rotation.x);
      if (delta_angle_pitch > distance_pitch) 
         delta_angle_pitch = new_heading.pitch - missile->rotation.x;

      missile->rotation.y += delta_angle_yaw   * yaw_dir;
      missile->rotation.x += delta_angle_pitch * pitch_dir;

      log(LOG_INFO, "rotation:" + to_string(vec2(missile->rotation.x, missile->rotation.y)));

      vec3 D = normalize(player->entity_ptr->position - missile->position);
      IM_RENDER.add_line(IMHASH, missile->position, missile->position + D * 2.f, 1.2f, false, COLOR_GREEN_1);

      editor_print(to_string(missile->rotation));
   }

   mat4 missile_rot  = rotate(mat4identity, glm::radians(missile->rotation.x), vec3(1.0f, 0.0f, 0.0f));
   missile_rot       = rotate(missile_rot,  glm::radians(missile->rotation.y), vec3(0.0f, 1.0f, 0.0f));
   missile_rot       = rotate(missile_rot,  glm::radians(missile->rotation.z), vec3(0.0f, 0.0f, 1.0f));

   vec3 heading = to_vec3(missile_rot * MISSILE_INITIAL_POS);
   IM_RENDER.add_line(IMHASH, missile->position, missile->position + heading * 2.f, 1.2f, false, COLOR_RED_1);

   if (UPDATE_MISSILE)
   {
      missile->velocity  = MISSILE_LIN_SPEED * heading; 
      missile->position  += missile->velocity * G_FRAME_INFO.duration;
   }
}


Heading look_at(Entity* entity, vec3 position)
{
   vec3 look_vec = normalize(position - entity->position);

	float pitch = glm::degrees(glm::asin(look_vec.y));
	float yaw   = glm::degrees(atan2(look_vec.x, -1 * look_vec.z) - PI / 2);

   vec3 heading = look_vec;
	// heading.x   = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	// heading.y   = sin(glm::radians(pitch));
	// heading.z   = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	// heading     = normalize(heading);

   IM_RENDER.add_line(IMHASH, entity->position, entity->position + heading * 2.f, 1.2f, false, COLOR_YELLOW_1);

   return Heading{pitch, yaw, heading};
}