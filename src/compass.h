

// mock API
vec3 get_orientation_from_api(Entity* entity)
{
   return vec3{entity->rotation.x, entity->rotation.y, entity->rotation.z};
}

// util math functions
// inline
// vec3 project_vec_into_ref(vec3 vec, vec3 ref)
// {
//    auto proj = dot(vec, ref) / (abs(ref) * abs(ref)) * ref;
//    return proj;
// }

// inline 
// vec3 project_vec_onto_plane(vec3 v, vec3 n)
// {
//    auto v_proj_n     = project_vec_into_ref(v, n);
//    auto v_proj_plane = v - v_proj_n;
//
//    return v_proj_plane;
// }

// inline
// float vector_angle(vec3 A, vec3 B)
// {
//    float dot = glm::dot(A, B);
//    float len_A = glm::length(A);
//    float len_B = glm::length(B);
//    float theta = acos(dot / (len_A * len_B));
//    return theta;
// }

float get_compass_heading(Entity* entity)
{
   // first get angles, assuming XYZ euler angles order, angles in degrees
   vec3 euler_angles = get_orientation_from_api(entity);

   float pitch, yaw, roll;
   pitch    = euler_angles.x;
   yaw      = euler_angles.y;
   roll     = euler_angles.z;

   // second, create reference vectors, assuming a compass display plane with normal towards the y axis (up) and
   // up vector pointing towards positive Z (representing north).
   vec3 north      = vec3(0, 0, 1);
   vec3 screen_n   = vec3(0, 1, 0);
   vec3 screen_up  = vec3(0, 0, 1);

   // second, get rotation matrix from euler angles
   mat4 rot    = glm::rotate(mat4identity,   glm::radians(pitch),    vec3(1, 0, 0));
   rot         = glm::rotate(rot,            glm::radians(yaw),      vec3(0, 1, 0));
   rot         = glm::rotate(rot,            glm::radians(roll),     vec3(0, 0, 1));

   // third, rotate the screen normal and up vectors to get screen orientation
   screen_n    = glm::normalize(to_vec3(rot * to_vec4(screen_n, 1) ));
   screen_up   = glm::normalize(to_vec3(rot * to_vec4(screen_up, 1)));

   // third, project the north vec vec3(0, 0, 1) onto the screen.
   float screen_north_dot = dot(north, screen_n);

   vec3 projected_north;
   // if(screen_north_dot == 0 || screen_north_dot == 1 || screen_north_dot == -1)
   // {
   //    projected_north = screen_up;
   // }
   // else
   {
      projected_north = glm::normalize(project_vec_onto_plane(north, screen_n));
   }

   // fourth, if northvec and screen normal are parallel, assume angle = 0 degrees.
   float angle = glm::degrees(vector_angle(screen_up, projected_north));

   vec3 original_center = vec3(
      entity->position.x + entity->scale.x / 2.0, 
      entity->position.y + entity->scale.y, 
      entity->position.z + entity->scale.z / 2.0
   );

   vec3 center_rot   = to_vec3(rot * to_vec4(original_center - entity->position, 1));
   vec3 center       = entity->position + center_rot;

   IM_RENDER.add_line(IMHASH, center, center + screen_up          * 1.5f, 3.0f, false, COLOR_BLUE_1);
   IM_RENDER.add_line(IMHASH, center, center + screen_n           * 1.5f, 3.0f, false, COLOR_GREEN_1);
   IM_RENDER.add_line(IMHASH, center, center + projected_north    * 0.8f, 3.0f, true,  COLOR_YELLOW_1);

   // IM_RENDER.add_line(IMHASH, center, center + projected_north * 2.f, 1.2f, false, COLOR_YELLOW_1);
   // IM_RENDER.add_line(IMHASH, center, center + projected_north * 2.f, 1.2f, false, COLOR_YELLOW_1);


   editor_print("Angle from north: " + to_string(angle));

   return angle;
}





