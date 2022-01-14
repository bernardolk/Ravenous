// --------------------
// SCENE OBJECTS PANEL
// --------------------

void render_scene_objects_panel(SceneObjectsPanelContext* panel)
{
   ImGui::SetNextWindowPos(ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH - 600, 50), ImGuiCond_Appearing);
   ImGui::Begin("Scene objects", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);

   ImGui::InputText("Search", &panel->search_text);
   ImGui::NewLine();

   // copy search text and lowercase it
   string _search_text;
   _search_text.assign(panel->search_text);
   tolower(&_search_text);

   Scene* scene = G_SCENE_INFO.active_scene;
   For(scene->entities.size())
   {
      Entity* entity = scene->entities[i];
      string name = entity->name;
      tolower(&name);

      if(panel->search_text == "" || name.find(_search_text) != std::string::npos)
      {
         if(ImGui::Button(entity->name.c_str(), ImVec2(200, 28)))
         {
            panel->active = false;
            
            if(entity->name == PLAYER_NAME)
               open_entity_panel(entity);
            else
               open_player_panel(G_SCENE_INFO.player);
         }
      }
   }

   ImGui::End();
}