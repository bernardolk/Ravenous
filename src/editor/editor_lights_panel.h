// -------------
// LIGHTS PANEL
// -------------
vec3 compute_direction_from_angles(float pitch, float yaw);
void compute_angles_from_direction(float& pitch, float& yaw, vec3 direction);


vec3 compute_direction_from_angles(float pitch, float yaw)
{
   vec3 arrow_direction;
   arrow_direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
   arrow_direction.y = sin(glm::radians(pitch));
   arrow_direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
   arrow_direction = glm::normalize(arrow_direction);
   return arrow_direction;
}


void compute_angles_from_direction(float& pitch, float& yaw, vec3 direction)
{
   pitch = glm::degrees(glm::asin(direction.y));
   yaw = glm::degrees(atan2(direction.x, -1 * direction.z) - 3.141592 / 2);
   if (pitch > 89.0f)  pitch = 89.0f;
   if (pitch < -89.0f) pitch = -89.0f;
   if (yaw > 360.0f)   yaw -= 360.0f;
   if (yaw < -360.0f)  yaw += 360.0f;
   return;
}


void render_lights_panel(LightsPanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
   ImGui::Begin("Lights Panel", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);

   ImGui::Text("Point Lights");
   for(int i = 0; i < G_SCENE_INFO.active_scene->pointLights.size(); i++)
   {
      string header = "light source (" + to_string(i) + ")";
      if(ImGui::CollapsingHeader(header.c_str()))
      {
         auto& light = G_SCENE_INFO.active_scene->pointLights[i];
         bool is_active = panel->selected_light == i;
         auto show_name = "show##" + to_string(i);
         if(ImGui::Checkbox(show_name.c_str(), &is_active))
         {
            panel->selected_light = is_active ? i : -1;
         }
         ImGui::NewLine();

         // position 
         float positions[]{ light.position[0], light.position[1], light.position[2] };
         auto label_pos = "position##" + to_string(i);
         if(ImGui::SliderFloat3(label_pos.c_str(), positions, -10.0, 10.0))
         {
            light.position = vec3{positions[0], positions[1], positions[2]};
         }

         // direction
         {
            float yaw, pitch;
            compute_angles_from_direction(pitch, yaw, light.direction);

            // pitch
            auto label_pitch = "pitch##" + to_string(i);
            if(ImGui::SliderFloat(label_pitch.c_str(), &pitch, -89.0, 89.0))
            {
               light.direction = compute_direction_from_angles(pitch, yaw);
            }

            //yaw
            auto label_yaw = "yaw##" + to_string(i);
            if(ImGui::SliderFloat(label_yaw.c_str(), &yaw, -360.0, 360.0))
            {
               light.direction = compute_direction_from_angles(pitch, yaw);
            }
         }

         // diffuse color 
         float diffuse[]{ light.diffuse[0], light.diffuse[1], light.diffuse[2] };
         auto label_diffuse = "diffuse##" + to_string(i);
         if(ImGui::ColorPicker3(label_diffuse.c_str(), diffuse, ImGuiColorEditFlags_NoAlpha))
         {
            light.diffuse = vec3{diffuse[0], diffuse[1], diffuse[2]};
         }

         // specular color 
         float specular[]{ light.specular[0], light.specular[1], light.specular[2] };
         auto label_specular = "specular##" + to_string(i);
         if(ImGui::ColorPicker3(label_specular.c_str(), specular, ImGuiColorEditFlags_NoAlpha))
         {
            light.specular = vec3{specular[0], specular[1], specular[2]};
         }

         // intensity
         float intensity[] { light.intensity_constant, light.intensity_linear, light.intensity_quadratic };
         auto label_intensity = "intensity##" + to_string(i);
         if(ImGui::SliderFloat3(label_intensity.c_str(), intensity, 0.0, 5.0))
         {
            light.intensity_constant  = intensity[0]; 
            light.intensity_linear    = intensity[1]; 
            light.intensity_quadratic = intensity[2]; 
         }

         ImGui::NewLine();
      }
   } 

   ImGui::End();
}