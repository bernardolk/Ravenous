#include "editor/EditorPlayerPanel.h"
#include <imgui.h>
#include "EditorMain.h"
#include "EditorPanelContexts.h"
#include "game/entities/EPlayer.h"
#include "engine/utils/utils.h"
#include "engine/io/display.h"

namespace Editor
{
	void RenderPlayerPanel(RPlayerPanelContext* Panel)
	{
		auto* Player = EPlayer::Get();

		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayState::ViewportWidth - 550, 370), ImGuiCond_Appearing);

		ImGui::Begin("Player Panel", &Panel->Active, ImGuiWindowFlags_AlwaysAutoResize);
		Panel->Focused = ImGui::IsWindowFocused();

		string EntityIdentification = Player->Name;
		ImGui::Text(EntityIdentification.c_str());

		string EntityId = "Id: " + std::to_string(Player->ID);
		ImGui::Text(EntityId.c_str());

		ImGui::NewLine();

		bool HideControl = Player->Flags & EntityFlags_HiddenEntity;
		if (ImGui::Checkbox("Hide Entity", &HideControl))
		{
			Player->Flags ^= EntityFlags_HiddenEntity;
		}
		ImGui::NewLine();

		ImGui::Text("Velocity: ");
		ImGui::Text(ToString(Player->Velocity).c_str());

		ImGui::End();
	}

	void OpenPlayerPanel(EPlayer* Player)
	{
		auto& EdContext = *GetContext();

		EdContext.SelectedEntity = Player;

		auto& Panel = EdContext.PlayerPanel;
		Panel.Active = true;
		Panel.Player = Player;
	}
}
