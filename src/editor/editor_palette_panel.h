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
         auto ref_entity = panel->entity_palette[i];
         auto new_entity = Entity_Manager.copy_entity(ref_entity);
         select_entity_to_move_with_mouse(new_entity);
      }
   }

   ImGui::End();
}


void initialize_palette(PalettePanelContext* panel)
{
   // 0
   auto sandstone_box = Entity_Manager.create_entity("NONAME", "aabb", "model", "sandstone");
   sandstone_box->collision_geometry_type = COLLISION_ALIGNED_BOX;
   panel->entity_palette[0] = sandstone_box;
   panel->count++;

   // 1
   auto sandstone_slope = Entity_Manager.create_entity("NONAME", "slope", "model", "sandstone");
   sandstone_slope->collision_geometry_type = COLLISION_ALIGNED_SLOPE;
   panel->entity_palette[1] = sandstone_slope;
   panel->count++;
}