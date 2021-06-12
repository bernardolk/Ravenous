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
         auto new_entity = copy_entity(ref_entity);
         G_SCENE_INFO.active_scene->entities.push_back(new_entity);
         select_entity_to_move_with_mouse(new_entity);
      }
   }

   ImGui::End();
}


void initialize_palette(PalettePanelContext* panel)
{
   auto sandstone_texture = Texture_Catalogue.find("sandstone")->second;
   auto model_shader = Shader_Catalogue.find("model")->second;

   // 0
   auto sandstone_box = new Entity();
   sandstone_box->shader = model_shader;
   sandstone_box->textures.push_back(sandstone_texture);
   sandstone_box->mesh = Geometry_Catalogue.find("aabb")->second;
   panel->entity_palette[0] = sandstone_box;
   panel->count++;

   // 1
   auto sandstone_slope = new Entity();
   sandstone_slope->shader = model_shader;
   sandstone_slope->textures.push_back(sandstone_texture);
   sandstone_slope->mesh = Geometry_Catalogue.find("slope")->second;
   panel->entity_palette[1] = sandstone_slope;
   panel->count++;
}