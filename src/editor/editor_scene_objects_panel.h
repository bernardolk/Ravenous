// --------------------
// SCENE OBJECTS PANEL
// --------------------

void render_scene_objects_panel(World* world, SceneObjectsPanelContext* panel)
{
	ImGui::SetNextWindowPos(ImVec2(GlobalDisplayConfig::VIEWPORT_WIDTH - 600, 50), ImGuiCond_Appearing);
	ImGui::Begin("Scene objects", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::InputText("Search", &panel->search_text);
	ImGui::NewLine();

	// copy search text and lowercase it
	std::string _search_text;
	_search_text.assign(panel->search_text);
	tolower(&_search_text);

	For(world->entities.size())
	{
		Entity* entity = world->entities[i];
		std::string name = entity->name;
		tolower(&name);

		if(panel->search_text == "" || name.find(_search_text) != std::string::npos)
		{
			if(ImGui::Button(entity->name.c_str(), ImVec2(200, 28)))
			{
				panel->active = false;

				if(entity->name == PlayerName)
					open_player_panel(GSceneInfo.player);
				else
					open_entity_panel(entity);
			}
		}
	}

	ImGui::End();
}
