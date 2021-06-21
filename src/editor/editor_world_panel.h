// -------------
// WORLD PANEL
// -------------

void render_world_panel(WorldPanelContext* panel, WorldStruct* world)
{
   ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
   ImGui::Begin("World Panel", &panel->active, ImGuiWindowFlags_None);
   ImGui::SetWindowSize("World Panel", ImVec2(500,700), ImGuiCond_Appearing);

   ImGui::Text("World Cells");
   for(int i = 0; i < world->cells_in_use_count; i++)
   {
      auto cell = world->cells_in_use[i];

      if(ImGui::CollapsingHeader(cell->coordinates_str().c_str()))
      {
         auto cell_coords = vec3{cell->i, cell->j, cell->k};
         bool is_active = panel->cell_coords == cell_coords;
         string show_name = "show##" + to_string(i);

         if(ImGui::Checkbox(show_name.c_str(), &is_active))
         {
            panel->cell_coords = is_active ? cell_coords : vec3{-1.0f};
         }

         for(int e_i = 0; e_i < WORLD_CELL_CAPACITY; e_i++)
         {
            auto entity = cell->entities[e_i];
            if(entity != nullptr)
            {
               string line = to_string(e_i + 1) + ". " + entity->name;
               ImGui::Text(line.c_str());
            }
         }
      }
   } 

   ImGui::End();
}