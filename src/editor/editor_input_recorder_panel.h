// ------------------------
// > INPUT RECORDER PANEL
// ------------------------


void render_input_recorder_panel(InputRecorderPanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH - 400, 800), ImGuiCond_Once);
   ImGui::Begin("Input Recorder Panel", &panel->active, ImGuiWindowFlags_None);
   ImGui::SetWindowSize("Input Recorder Panel", ImVec2(350, 220), ImGuiCond_Always);

   ImGui::Text("Recordings");
   for(int i = 0; i < Input_Recorder.recording_idx; i++)
   {
      string rec_name = "Recording #" + to_string(i);
      bool is_active = panel->selected_recording == i;
      if(ImGui::Checkbox(rec_name.c_str(), &is_active))
      {
         panel->selected_recording = i;
      }
   }

   if(Input_Recorder.recording_idx == 0)
   {
      ImGui::NewLine();
      ImGui::Text("No recordings yet.");
      ImGui::NewLine();
      ImGui::NewLine();
   }

   if(ImGui::Button("Record", ImVec2(60,18)))
   {
      RENDER_MESSAGE("Input Recording Started", 2000);
      Input_Recorder.start_recording();
   }

   ImGui::SameLine();
   if(Input_Recorder.is_recording)
   {
      if(ImGui::Button("Stop Rec", ImVec2(60,18)))
      {
         RENDER_MESSAGE("Input Recording Stoped", 2000);
         Input_Recorder.stop_recording();
      }
   }

   if(Input_Recorder.recording_idx > 0)
   {
      if(ImGui::Button("Play", ImVec2(60,18)))
      {
         Input_Recorder.start_playing(panel->selected_recording);
      }
   }

   ImGui::SameLine();
   if(Input_Recorder.is_playing)
   {
      if(ImGui::Button("Stop", ImVec2(60,18)))
      {
        Input_Recorder.stop_playing();
      }
   }

   ImGui::End();
}