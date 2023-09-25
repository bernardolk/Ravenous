#include "EditorWorldPanel.h"
#include <imgui.h>
#include "EditorPanelContexts.h"
#include "engine/world/World.h"
#include "game/entities/EPlayer.h"

namespace Editor
{
	void RenderWorldPanel(RWorldPanelContext* Panel, const RWorld* World, const EPlayer* Player)
	{
		ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_Appearing);
		ImGui::Begin("World Panel", &Panel->Active, ImGuiWindowFlags_None);
		ImGui::SetWindowSize("World Panel", ImVec2(500, 700), ImGuiCond_Appearing);

		ImGui::Text("World Cells");
		int i = 0;
		for (auto* ActiveChunk : World->ActiveChunks)
		{
			auto ChunkPosition = ActiveChunk->GetChunkPosition();
			// adds indicator in header if player is inside cell
			string Header = ActiveChunk->GetChunkPositionString();
			for (auto* PlayerChunk : Player->WorldChunks)
			{
				if (ChunkPosition == PlayerChunk->GetChunkPosition())
				{
					Header += " P";
					break;
				}
			}

			if (ImGui::CollapsingHeader(Header.c_str()))
			{
				bool IsActive = Panel->ChunkPositionVec == ActiveChunk->GetChunkPosition();
				if (ImGui::Checkbox(string("show##" + to_string(i)).c_str(), &IsActive))
				{
					Panel->ChunkPositionVec = IsActive ? ChunkPosition.GetVec() : vec3{-1.0f};
				}

				ImGui::SameLine();

				string CoordsMeters = ActiveChunk->GetChunkPositionMetricString();
				ImGui::Text(CoordsMeters.c_str());

				auto EntityIter = ActiveChunk->GetIterator();
				int EntityCount = 1;
				while (auto* Entity = EntityIter())
				{
					string Line = to_string(EntityCount++) + ". " + Entity->name;
					ImGui::Text(Line.c_str());
				}
			}
			i++;
		}

		ImGui::End();
	}
}
