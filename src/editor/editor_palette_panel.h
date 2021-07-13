// --------------
// PALETTE PANEL
// --------------

void render_palette_panel(PalettePanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(50, 300), ImGuiCond_Always);
   ImGui::Begin("Palette", &panel->active, ImGuiWindowFlags_NoResize);
   ImGui::SetWindowSize("Palette", ImVec2(90, 500), ImGuiCond_Always);

   for(unsigned int i = 0; i < panel->count; i++)
   {
      if(ImGui::ImageButton((void*)(intptr_t)panel->textures[i], ImVec2(64,64)))
      {
         auto attributes = panel->entity_palette[i];
         auto new_entity = Entity_Manager.create_entity(&attributes);
         new_entity->position = G_SCENE_INFO.camera->Position + (2.f * new_entity->scale + 5.f) * G_SCENE_INFO.camera->Front;
         activate_move_mode(new_entity);
      }
   }

   ImGui::End();
}


void initialize_palette(PalettePanelContext* panel)
{
   int texture_count = 0;
   panel->textures[texture_count++] = load_texture_from_file("box.png", EDITOR_ASSETS);
   panel->textures[texture_count++] = load_texture_from_file("slope.png", EDITOR_ASSETS);
   panel->textures[texture_count++] = load_texture_from_file("checkpoint.png", EDITOR_ASSETS);

   // 0
   panel->entity_palette[panel->count++] = EntityAttributes{
      "NONAME", 
      "aabb", 
      "model", 
      "grey", 
      COLLISION_ALIGNED_BOX,
      STATIC
   };

   // 1
   panel->entity_palette[panel->count++] = EntityAttributes{
      "NONAME", 
      "slope", 
      "model", 
      "grey", 
      COLLISION_ALIGNED_SLOPE,
      STATIC
   };

   // 3
   panel->entity_palette[panel->count++] = EntityAttributes{
      "NONAME-CHECKPOINT", 
      "aabb", 
      "model", 
      "grey", 
      COLLISION_ALIGNED_BOX,
      CHECKPOINT,
      vec3(0.3, 1.2, 0.3)
   };
}