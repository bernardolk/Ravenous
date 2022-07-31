// ---------------
// > WORLD PANEL
// ---------------

void render_world_panel(WorldPanelContext* panel, World* world, Player* player)
{
   ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
   ImGui::Begin("World Panel", &panel->active, ImGuiWindowFlags_None);
   ImGui::SetWindowSize("World Panel", ImVec2(500,700), ImGuiCond_Appearing);

   ImGui::Text("World Cells");
   for(int i = 0; i < world->cells_in_use_count; i++)
   {
      auto cell = world->cells_in_use[i];
      auto coords = cell->coords();

      // adds indicator in header if player is inside cell
     std::string player_indicator = "";
      for(int i = 0; i < player->entity_ptr->world_cells_count; i++)
      {
         if(coords == player->entity_ptr->world_cells[i]->coords())
         {
            player_indicator = " P";
            break;
         }
      }

     std::string header = cell->coords_str() + player_indicator;
      if(ImGui::CollapsingHeader(header.c_str()))
      {
         bool is_active = panel->cell_coords == coords;
         std::string show_name = "show##" + std::to_string(i);
         if(ImGui::Checkbox(show_name.c_str(), &is_active))
         {
            panel->cell_coords = is_active ? coords : vec3{-1.0f};
         }

         ImGui::SameLine();

         std::string coords_meters = cell->coords_meters_str();
         ImGui::Text(coords_meters.c_str());

         for(int e_i = 0; e_i < WORLD_CELL_CAPACITY; e_i++)
         {
            auto entity = cell->entities[e_i];
            if(entity != nullptr)
            {
               std::string line = std::to_string(e_i + 1) + ". " + entity->name;
               ImGui::Text(line.c_str());
            }
            else
               break;
         }
      }
   } 

   ImGui::End();
}