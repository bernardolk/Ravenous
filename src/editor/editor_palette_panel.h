// --------------
// PALETTE PANEL
// --------------

void render_palette_panel(PalettePanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(50, 300), ImGuiCond_Always);
   ImGui::Begin("Palette", &panel->active, ImGuiWindowFlags_NoResize);
   ImGui::SetWindowSize("Palette", ImVec2(150,700), ImGuiCond_Appearing);

   // box
   if(ImGui::ImageButton((void*)(intptr_t)panel->textures[0], ImVec2(64,64)))
   {

   }

   // slope
   if(ImGui::ImageButton((void*)(intptr_t)panel->textures[1], ImVec2(64,64)))
   {

   }

   ImGui::End();
}