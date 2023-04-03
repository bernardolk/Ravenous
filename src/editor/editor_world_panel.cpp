#include "editor_world_panel.h"
#include <imgui.h>
#include "editor_panel_contexts.h"
#include "game/entities/player.h"
#include "engine/world/world.h"

namespace Editor
{
	void RenderWorldPanel(WorldPanelContext* panel, const T_World* world, const Player* player)
	{
		ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
		ImGui::Begin("World Panel", &panel->active, ImGuiWindowFlags_None);
		ImGui::SetWindowSize("World Panel", ImVec2(500, 700), ImGuiCond_Appearing);

		ImGui::Text("World Cells");
		int i = 0;
		for (auto* active_chunk : world->active_chunks)
		{
			auto coords = active_chunk->GetChunkPosition();
			// adds indicator in header if player is inside cell
			string header = active_chunk->GetChunkPositionString();
			for (auto* player_chunk : player->world_chunks)
			{
				if (coords == player_chunk->GetChunkPosition())
				{
					header += " P";
					break;
				}
			}

			if (ImGui::CollapsingHeader(header.c_str()))
			{
				bool is_active = panel->cell_coords == active_chunk->GetChunkPosition();
				if (ImGui::Checkbox(string( "show##" + to_string(i)).c_str(), &is_active))
				{
					panel->cell_coords = is_active ? coords : vec3{-1.0f};
				}

				ImGui::SameLine();

				string coords_meters = active_chunk->GetChunkPositionMetricString();
				ImGui::Text(coords_meters.c_str());

				auto entity_iter = active_chunk->GetIterator();
				int entity_count = 1;
				while(auto* entity = entity_iter())
				{
					string line = to_string(entity_count++) + ". " + entity->name;
					ImGui::Text(line.c_str());
				}
			}
			i++;
		}

		ImGui::End();
	}
}
