#include "editor/EditorPlayerPanel.h"
#include <imgui.h>

#include "editor.h"
#include "EditorPanelContexts.h"
#include "game/entities/EPlayer.h"
#include "engine/utils/utils.h"
#include "engine/io/display.h"

namespace Editor
{
	void RenderPlayerPanel(RPlayerPanelContext* panel)
	{
		auto* player = EPlayer::Get();

		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayState::viewport_width - 550, 370), ImGuiCond_Appearing);

		ImGui::Begin("Player Panel", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);
		panel->focused = ImGui::IsWindowFocused();

		std::string entity_identification = player->name;
		ImGui::Text(entity_identification.c_str());

		std::string entity_id = "Id: " + std::to_string(player->id);
		ImGui::Text(entity_id.c_str());

		ImGui::NewLine();

		bool _hide_control = player->flags & EntityFlags_HiddenEntity;
		if (ImGui::Checkbox("Hide Entity", &_hide_control))
		{
			player->flags ^= EntityFlags_HiddenEntity;
		}
		ImGui::NewLine();

		ImGui::Text("Velocity: ");
		ImGui::Text(ToString(player->velocity).c_str());

		ImGui::End();
	}

	void OpenPlayerPanel(EPlayer* player)
	{
		auto& ed_context = *Editor::GetContext();

		ed_context.selected_entity = player;

		auto& panel = ed_context.player_panel;
		panel.active = true;
		panel.player = player;
	}
}
