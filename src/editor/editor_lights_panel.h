// -------------
// LIGHTS PANEL
// -------------
void render_lights_panel(LightsPanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
   ImGui::Begin("Lights Panel", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);

   ImGui::Text("Point Lights");
   for(int i = 0; i < G_SCENE_INFO.active_scene->pointLights.size(); i++)
   {
      auto light = G_SCENE_INFO.active_scene->pointLights[i];
      string header = "light source (" + to_string(i) + ")";
      if(ImGui::CollapsingHeader(header.c_str()))
      {
         bool is_active = panel->selected_light == i;
         string show_name = "show##" + to_string(i);
         if(ImGui::Checkbox(show_name.c_str(), &is_active))
         {
            panel->selected_light = is_active ? i : -1;
         }
         ImGui::NewLine();
      }
   } 

   ImGui::End();
}