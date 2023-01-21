// -------------
// PLAYER PANEL
// -------------

void render_player_panel(PlayerPanelContext* panel);
void open_player_panel(Player* player);


void render_player_panel(PlayerPanelContext* panel)
{
	auto& entity = panel->player->entity_ptr;
	auto& player = panel->player;

	ImGui::SetNextWindowPos(ImVec2(GlobalDisplayConfig::VIEWPORT_WIDTH - 550, 370), ImGuiCond_Appearing);

	ImGui::Begin("Player Panel", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);
	panel->focused = ImGui::IsWindowFocused();

	std::string entity_identification = entity->name;
	ImGui::Text(entity_identification.c_str());

	std::string entity_id = "Id: " + std::to_string(entity->id);
	ImGui::Text(entity_id.c_str());

	ImGui::NewLine();

	bool _hide_control = entity->flags & EntityFlags_HiddenEntity;
	if(ImGui::Checkbox("Hide Entity", &_hide_control))
	{
		entity->flags ^= EntityFlags_HiddenEntity;
	}
	ImGui::NewLine();

	ImGui::Text("Speed: ");
	ImGui::Text(fmt_tostr(player->speed, 3).c_str());
	ImGui::NewLine();

	ImGui::Text("Velocity: ");
	ImGui::Text(to_string(player->entity_ptr->velocity).c_str());

	ImGui::End();
}

void open_player_panel(Player* player)
{
	EdContext.selected_entity = player->entity_ptr;

	auto& panel = EdContext.player_panel;
	panel.active = true;
	panel.player = player;
}
