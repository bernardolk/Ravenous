// -------------
// WORLD PANEL
// -------------

void render_world_panel(WorldPanelContext* panel, World* world)
{
   ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
   ImGui::Begin("World Panel", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);

   ImGui::Text("World Cells");
   for(int i = 0; i < world->cells_in_use_count; i++)
   {
      auto cell = world->cells_in_use[i];
      string name = "Cell [" + to_string(cell->i) + "," + to_string(cell->j) + "," + to_string(cell->k) + "]";
      if(ImGui::CollapsingHeader(name.c_str()))
      {
         for(int e_i = 0; e_i < WORLD_CELL_CAPACITY; e_i++)
         {
            auto entity = cell->entities[e_i];
            if(entity != nullptr)
               ImGui::Text(entity->name.c_str());
            else
               ImGui::Text("NULL");
         }
      }
   } 

   ImGui::End();
}






