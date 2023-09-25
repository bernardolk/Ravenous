#include "EditorCollisionLogPanel.h"
#include <imgui.h>
#include "EditorPanelContexts.h"
#include "engine/io/display.h"

namespace Editor
{
	void RenderCollisionLogPanel(RCollisionLogPanelContext* Panel)
	{
		constexpr uint16 WWidth = 450;
		constexpr uint16 WHeight = 320;
		constexpr uint8 WBottomMargin = 30;
		constexpr uint8 TableItemsToShow = 100;
		constexpr uint16 TableHeight = 120;

		ImGui::SetNextWindowPos(
			ImVec2(GlobalDisplayState::viewport_width - w_width, GlobalDisplayState::viewport_height - w_height - w_bottom_margin),
			ImGuiCond_Appearing
		);

		ImGui::Begin("Collision Log Panel", &Panel->Active, ImGuiWindowFlags_None);
		ImGui::SetWindowSize("Collision Log Panel", ImVec2(WWidth, WHeight), ImGuiCond_Once);
		Panel->Focused = ImGui::IsWindowFocused();

		ImGuiTableFlags Flags =
		ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
		ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

		PushStyleCompact();
		ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &Flags, ImGuiTableFlags_ScrollY);
		PopStyleCompact();

		// When using ScrollX or ScrollY we need to specify a size for our table container!
		// Otherwise by default the table will fit all available space, like a BeginChild() call.
		auto OuterSize = ImVec2(0.0f, TableHeight);
		if (ImGui::BeginTable("table_scrolly", 4, Flags, OuterSize))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn(".", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Entity", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Penetration", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Normal", ImGuiTableColumnFlags_None);
			ImGui::TableHeadersRow();

			ImGuiListClipper Clipper;
			Clipper.Begin(TableItemsToShow);
			while (Clipper.Step())
			{
				// for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				// {
				//    CollisionLogEntry* entry = CL_read_collision_log_entry(row);
				//    if(entry == nullptr || entry->entity == NULL)
				//       break;

				//    ImGui::TableNextRow();
				//    ImGui::TableSetColumnIndex(0);
				//    ImGui::Text(std::to_string(row).c_str());
				//    ImGui::TableSetColumnIndex(1);
				//    ImGui::Text(entry->entity->name.c_str());
				//    ImGui::TableSetColumnIndex(2);
				//    ImGui::Text(std::to_string(entry->penetration).c_str());
				//    ImGui::TableSetColumnIndex(3);
				//    ImGui::Text(to_string(entry->normal).c_str());
				// }
			}
			ImGui::EndTable();
		}

		ImGui::End();
	}

	void PushStyleCompact()
	{
		ImGuiStyle& Style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(Style.FramePadding.x, static_cast<float>((int)(Style.FramePadding.y * 0.60f))));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Style.ItemSpacing.x, static_cast<float>((int)(Style.ItemSpacing.y * 0.60f))));
	}

	void PopStyleCompact()
	{
		ImGui::PopStyleVar(2);
	}
}
