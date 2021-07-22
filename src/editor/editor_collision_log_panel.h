// --------------------
// COLLISION LOG PANEL
// --------------------

void render_collision_log_panel(CollisionLogPanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(180, 80), ImGuiCond_Appearing);
   ImGui::Begin("Collision Log Panel", &panel->active, ImGuiWindowFlags_None);
   ImGui::SetWindowSize("Collision Log Panel", ImVec2(330, 900), ImGuiCond_Always);
   panel->focused = ImGui::IsWindowFocused();

   // render a log type of window here
   int i = 0;
   while(true)
   {
      CollisionLogEntry* entry = read_collision_log_entry(i++);
      if(entry == nullptr || entry->entity == NULL)
         break;
      
      ImGui::Text((to_string(i) + ". " + entry->entity->name).c_str());
   }

   ImGui::Text(to_string(COLLISION_LOG->write_count).c_str());

   ImGui::End();
}




