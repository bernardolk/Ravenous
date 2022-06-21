// ------------------------
// > INPUT RECORDER PANEL
// ------------------------

void render_input_recorder_panel(InputRecorderPanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(GlobalDisplayConfig::VIEWPORT_WIDTH - 400, 800), ImGuiCond_Once);
   ImGui::Begin("Input Recorder Panel", &panel->active, ImGuiWindowFlags_None);
   ImGui::SetWindowSize("Input Recorder Panel", ImVec2(350, 220), ImGuiCond_Always);

   ImGui::Text("Recordings");
   for(int i = 0; i < Input_Recorder.recording_idx; i++)
   {
      std::string rec_name = "Recording #" + std::to_string(i);
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

   if(!(Input_Recorder.is_recording || Input_Recorder.is_playing))
   {
      ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.03f, 0.6f, 0.6f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.03f, 0.7f, 0.7f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.03f, 0.8f, 0.8f));
      if(ImGui::Button("Record", ImVec2(60, 18)))
      {
         editor_print("Input Recording Started", 2000);
         Input_Recorder.start_recording();
      }
      ImGui::PopStyleColor(3);
   }

   if(Input_Recorder.is_recording)
   {
      if(ImGui::Button("Stop Recording", ImVec2(100, 18)))
      {
         editor_print("Input Recording Stoped", 2000);
         Input_Recorder.stop_recording();
      }
   }

   if(Input_Recorder.recording_idx > 0 && !(Input_Recorder.is_recording || Input_Recorder.is_playing))
   {
      ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.42f, 0.6f, 0.6f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.42f, 0.7f, 0.7f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.42f, 0.8f, 0.8f));
      if(ImGui::Button("Play", ImVec2(60,18)))
      {
         Input_Recorder.start_playing(panel->selected_recording);
      }
      ImGui::PopStyleColor(3);
   }

   if(Input_Recorder.is_playing)
   {
      if(ImGui::Button("Stop Playing", ImVec2(100, 18)))
      {
        Input_Recorder.stop_playing();
      }
   }

   ImGui::End();
}