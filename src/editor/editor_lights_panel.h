// -------------
// LIGHTS PANEL
// -------------
void open_lights_panel(string type = "", int index = -1, bool focus_tab = false)
{
   EdContext.lights_panel.active = true;
   if(type != "" && index > -1)
   {
      EdContext.lights_panel.selected_light = index;
      EdContext.lights_panel.selected_light_type = type;
   }
   if(focus_tab)
      EdContext.lights_panel.focus_tab = true;
}


vec3 compute_direction_from_angles(float pitch, float yaw)
{
   vec3 arrow_direction;
   arrow_direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
   arrow_direction.y = sin(glm::radians(pitch));
   arrow_direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
   arrow_direction = glm::normalize(arrow_direction);
   return arrow_direction;
}


void render_lights_panel(LightsPanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(180, 80), ImGuiCond_Appearing);
   ImGui::Begin("Lights Panel", &panel->active, ImGuiWindowFlags_None);
   ImGui::SetWindowSize("Lights Panel", ImVec2(330, 900), ImGuiCond_Always);
   panel->focused = ImGui::IsWindowFocused();

   ImGui::BeginTabBar("Types");

   // -------------
   // POINT LIGHTS
   // -------------
   auto point_flags = (panel->focus_tab && panel->selected_light_type == "point") ? 
      ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
   if(ImGui::BeginTabItem("Point Lights", NULL, point_flags))
   {
      int deleted_light_index = -1;
      auto pointlights = &G_SCENE_INFO.active_scene->pointLights;

      // UNFOCUS TAB
      if(point_flags == ImGuiTabItemFlags_SetSelected) panel->focus_tab = false;

      // ADD BUTTON
      ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.42f, 0.6f, 0.6f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.42f, 0.7f, 0.7f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.42f, 0.8f, 0.8f));
      if(ImGui::Button("Add new##point"))
      {
         // create new spotlight
         PointLight new_pointlight;
         pointlights->push_back(new_pointlight);
         activate_move_light_mode("point", pointlights->size() - 1);
      }
      ImGui::PopStyleColor(3);

      for(int i = 0; i < pointlights->size(); i++)
      {
         string header = "point light source (" + to_string(i) + ")";
         bool is_active = panel->selected_light == i && panel->selected_light_type == "point";
         if(ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_NoAutoOpenOnLog) || is_active)
         {
            // SHOW BUTTON
            auto& light = (*pointlights)[i];
            auto show_name = "show##point" + to_string(i);
            if(ImGui::Checkbox(show_name.c_str(), &is_active))
            {
               panel->selected_light = is_active ? i : -1;
               panel->selected_light_type = "point";
            }

            // DELETE BUTTON
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.03f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.03f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.03f, 0.8f, 0.8f));
            auto delete_btn_label = "Delete##point" + to_string(i);
            if(ImGui::Button(delete_btn_label.c_str()))
            {
               deleted_light_index = i;
            }
            ImGui::PopStyleColor(3);

            // position 
            ImGui::NewLine();
            float positions[]{ light.position[0], light.position[1], light.position[2] };
            auto label_pos = "position##point" + to_string(i);
            if(ImGui::DragFloat3(label_pos.c_str(), positions, 0.3, -10.0, 10.0))
            {
               light.position = vec3{positions[0], positions[1], positions[2]};
            }

            // diffuse color 
            float diffuse[]{ light.diffuse[0], light.diffuse[1], light.diffuse[2] };
            auto label_diffuse = "diffuse##point" + to_string(i);
            if(ImGui::ColorPicker3(label_diffuse.c_str(), diffuse, ImGuiColorEditFlags_NoAlpha))
            {
               light.diffuse = vec3{diffuse[0], diffuse[1], diffuse[2]};
            }

            // specular color 
            float specular[]{ light.specular[0], light.specular[1], light.specular[2] };
            auto label_specular = "specular##point" + to_string(i);
            if(ImGui::ColorPicker3(label_specular.c_str(), specular, ImGuiColorEditFlags_NoAlpha))
            {
               light.specular = vec3{specular[0], specular[1], specular[2]};
            }

            // intensity (decay)
            ImGui::Text("Intensity decay");
            auto label_intensity_const = "const##point" + to_string(i);
            ImGui::DragFloat(label_intensity_const.c_str(), &light.intensity_constant, 0.05, 0.0, 5.0);

            auto label_intensity_linear = "linear##point" + to_string(i);
            ImGui::DragFloat(label_intensity_linear.c_str(), &light.intensity_linear, 0.005, 0.0, 5.0);

            auto label_intensity_quad = "quadratic##point" + to_string(i);
            ImGui::DragFloat(label_intensity_quad.c_str(), &light.intensity_quadratic, 0.0005, 0.0, 5.0);

            ImGui::NewLine();
         }
      }

      if(deleted_light_index > -1)
         editor_erase_light(deleted_light_index, "point");

      ImGui::EndTabItem();
   }

   // ------------
   // SPOT LIGHTS
   // ------------
   auto spot_flags = (panel->focus_tab && panel->selected_light_type == "spot") ? 
      ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
   if(ImGui::BeginTabItem("Spot Lights", NULL, spot_flags))
   {
      int deleted_light_index = -1;
      auto spotlights = &G_SCENE_INFO.active_scene->spotLights;

      // UNFOCUS TAB
      if(spot_flags == ImGuiTabItemFlags_SetSelected) panel->focus_tab = false;

      // ADD BUTTON
      ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.42f, 0.6f, 0.6f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.42f, 0.7f, 0.7f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.42f, 0.8f, 0.8f));
      if(ImGui::Button("Add new##spot"))
      {
         // create new spotlight
         SpotLight new_spotlight;
         spotlights->push_back(new_spotlight);
         activate_move_light_mode("spot", spotlights->size() - 1);
      }
      ImGui::PopStyleColor(3);

      for(int i = 0; i < spotlights->size(); i++)
      {
         string header = "spot light source (" + to_string(i) + ")";
         bool is_active = panel->selected_light == i && panel->selected_light_type == "spot";
         if(ImGui::CollapsingHeader(header.c_str()) || is_active)
         {
            auto& light = (*spotlights)[i];

            // SHOW BUTTON
            auto show_name = "show##spot" + to_string(i);
            if(ImGui::Checkbox(show_name.c_str(), &is_active))
            {
               panel->selected_light = is_active ? i : -1;
               panel->selected_light_type = "spot";
            }

            // DELETE BUTTON
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.03f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.03f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.03f, 0.8f, 0.8f));
            auto delete_btn_spot_label = "Delete##spot" + to_string(i);
            if(ImGui::Button(delete_btn_spot_label.c_str()))
            {
               deleted_light_index = i;
            }
            ImGui::PopStyleColor(3);

            ImGui::NewLine();

            // position 
            float positions[]{ light.position[0], light.position[1], light.position[2] };
            auto label_pos = "position##spot" + to_string(i);
            if(ImGui::DragFloat3(label_pos.c_str(), positions, 0.3, -10.0, 10.0))
            {
               light.position = vec3{positions[0], positions[1], positions[2]};
            }

            // cones 
            auto label_innercone = "innercone##spot" + to_string(i);
            ImGui::DragFloat(label_innercone.c_str(), &light.innercone, 0.001, 1, MAX_FLOAT);

            auto label_outercone = "outercone##spot" + to_string(i);
            ImGui::DragFloat(label_outercone.c_str(), &light.outercone, 0.001, 0, 1);

            // direction
            {
               float yaw, pitch;
               compute_angles_from_direction(pitch, yaw, light.direction);

               // pitch
               auto label_pitch = "pitch##spot" + to_string(i);
               if(ImGui::SliderFloat(label_pitch.c_str(), &pitch, -89.0, 89.0))
               {
                  light.direction = compute_direction_from_angles(pitch, yaw);
               }

               //yaw
               auto label_yaw = "yaw##spot" + to_string(i);
               if(ImGui::SliderFloat(label_yaw.c_str(), &yaw, -360.0, 360.0))
               {
                  light.direction = compute_direction_from_angles(pitch, yaw);
               }
            }

            // diffuse color 
            float diffuse[]{ light.diffuse[0], light.diffuse[1], light.diffuse[2] };
            auto label_diffuse = "diffuse##spot" + to_string(i);
            if(ImGui::ColorPicker3(label_diffuse.c_str(), diffuse, ImGuiColorEditFlags_NoAlpha))
            {
               light.diffuse = vec3{diffuse[0], diffuse[1], diffuse[2]};
            }

            // specular color 
            float specular[]{ light.specular[0], light.specular[1], light.specular[2] };
            auto label_specular = "specular##spot" + to_string(i);
            if(ImGui::ColorPicker3(label_specular.c_str(), specular, ImGuiColorEditFlags_NoAlpha))
            {
               light.specular = vec3{specular[0], specular[1], specular[2]};
            }

            // intensity
            // intensity (decay)
            ImGui::Text("Intensity decay");
            auto label_intensity_const = "const##spot" + to_string(i);
            ImGui::DragFloat(label_intensity_const.c_str(), &light.intensity_constant, 0.05, 0.0, 5.0);

            auto label_intensity_linear = "linear##spot" + to_string(i);
            ImGui::DragFloat(label_intensity_linear.c_str(), &light.intensity_linear, 0.005, 0.0, 5.0);

            auto label_intensity_quad = "quadratic##spot" + to_string(i);
            ImGui::DragFloat(label_intensity_quad.c_str(), &light.intensity_quadratic, 0.0005, 0.0, 5.0);


            ImGui::NewLine();
         }
      }

      if(deleted_light_index > -1) 
         editor_erase_light(deleted_light_index, "spot");

      ImGui::EndTabItem();
   }

    ImGui::EndTabBar();

   ImGui::End();
}