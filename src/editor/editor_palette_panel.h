// --------------
// PALETTE PANEL
// --------------

void render_palette_panel(PalettePanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(50, 300), ImGuiCond_Always);
   ImGui::Begin("Palette", &panel->active, ImGuiWindowFlags_NoResize);
   ImGui::SetWindowSize("Palette", ImVec2(150,700), ImGuiCond_Appearing);

   for(unsigned int i = 0; i < panel->count; i++)
   {
      if(ImGui::ImageButton((void*)(intptr_t)panel->textures[i], ImVec2(64,64)))
      {
         auto attributes = panel->entity_palette[i];
         auto new_entity = Entity_Manager.create_entity(&attributes);
         select_entity_to_move_with_mouse(new_entity);
      }
   }

   ImGui::End();
}


void initialize_palette(PalettePanelContext* panel)
{
   panel->textures[0] = load_texture_from_file("box.png", EDITOR_ASSETS);
   panel->textures[1] = load_texture_from_file("slope.png", EDITOR_ASSETS);

   // 0
   panel->entity_palette[panel->count++] = EntityAttributes{
      "NONAME", 
      "aabb", 
      "model", 
      "sandstone", 
      COLLISION_ALIGNED_BOX
   };

   // 1
   panel->entity_palette[panel->count++] = EntityAttributes{
      "NONAME", 
      "slope", 
      "model", 
      "sandstone", 
      COLLISION_ALIGNED_SLOPE
   };
}