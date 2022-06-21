//------------------
// > EDITOR TOOLBAR
//------------------

void render_toolbar()
{
   ImGui::SetNextWindowPos(ImVec2(GlobalDisplayConfig::VIEWPORT_WIDTH - 230, 180), ImGuiCond_Appearing);
   ImGui::Begin("Tools", &EdContext.toolbar_active, ImGuiWindowFlags_AlwaysAutoResize);

  std::string scene_name = "Scene name: " + G_SCENE_INFO.scene_name;
   ImGui::Text(scene_name.c_str());
   ImGui::NewLine();

   ImGui::InputFloat("##timestep", &G_FRAME_INFO.time_step, 0.5, 1.0, "Timestep = %.1f x");

   ImGui::NewLine();

   // GLOBAL CONFIGS
   {
      bool track = false;

      ImGui::Text("Cam speed");
      ImGui::DragFloat("##camspeed", &G_SCENE_INFO.camera->Acceleration, 0.1, 0.2, MAX_FLOAT);
      track = track || ImGui::IsItemDeactivatedAfterEdit();
      
      // Ambient light control
      ImGui::Text("Ambient light");
      auto ambient = G_SCENE_INFO.active_scene->ambient_light;
      float colors[3] = { ambient.x, ambient.y, ambient.z};
      if(ImGui::ColorEdit3("##ambient-color", colors))
      {
         G_SCENE_INFO.active_scene->ambient_light = vec3{colors[0], colors[1], colors[2]};
      }
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      ImGui::SliderFloat("##ambient-intensity", &G_SCENE_INFO.active_scene->ambient_intensity, 0, 1, "intensity = %.2f");
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      // save to file changes in config variables
      if(track)
      {
         G_CONFIG.camspeed = G_SCENE_INFO.camera->Acceleration;
         G_CONFIG.ambient_intensity = G_SCENE_INFO.active_scene->ambient_intensity;
         G_CONFIG.ambient_light = G_SCENE_INFO.active_scene->ambient_light;
         save_configs_to_file();
      }

      ImGui::NewLine();
   }

   // PANELS
   if(ImGui::Button("Scene objects", ImVec2(150,18)))
   {
      EdContext.scene_objects_panel.active = true;
   }

   if(ImGui::Button("Entity Palette", ImVec2(150,18)))
   {
      EdContext.palette_panel.active = true;
   }

   if(ImGui::Button("World Panel", ImVec2(150,18)))
   {
      EdContext.world_panel.active = true;
      EdContext.show_world_cells = true;
   }

   if(ImGui::Button("Lights Panel", ImVec2(150,18)))
   {
      EdContext.lights_panel.active = true;
      EdContext.show_lightbulbs = true;
   }

   if(ImGui::Button("Input Recorder", ImVec2(150,18)))
   {
      EdContext.input_recorder_panel.active = true;
   }

   ImGui::NewLine();

   // TOOLS
   ImGui::Text("Measure");
   if(ImGui::Button("X", ImVec2(40,18)))
   {
      activate_measure_mode(0);
   }

   ImGui::SameLine();
   if(ImGui::Button("Y", ImVec2(40,18)))
   {
      activate_measure_mode(1);
   }
   
   ImGui::SameLine();
   if(ImGui::Button("Z", ImVec2(40,18)))
   {
      activate_measure_mode(2);
   }
   
   ImGui::NewLine();
   if(ImGui::Button("Locate Coordinates", ImVec2(150,18)))
   {
      activate_locate_coords_mode();
   }


   // SHOW STUFF
   ImGui::Checkbox("Show Event Triggers", &EdContext.show_event_triggers);
   ImGui::Checkbox("Show WorldStruct Cells", &EdContext.show_world_cells);
   ImGui::Checkbox("Show Point Lights", &EdContext.show_lightbulbs);
   ImGui::NewLine();
   if(ImGui::Button("Unhide entities"))
   {
      unhide_entities();
   }

   // OPTIONS
   if(ImGui::Button("Collision Logger", ImVec2(150,18)))
   {
      EdContext.collision_log_panel.active = true;
   }

   // DEBUG OPTIONS
   ImGui::Text("Debug options");
   ImGui::Checkbox("Ledge detection", &EdContext.debug_ledge_detection);

   ImGui::End();
}