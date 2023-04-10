#include "editor_scene_objects_panel.h"
#include <imgui.h>
#include "editor_entity_panel.h"
#include "editor_panel_contexts.h"
#include "editor_player_panel.h"
#include "game/entities/player.h"
#include "engine/io/display.h"
#include "engine/utils/utils.h"
#include "engine/world/world_chunk.h"

namespace Editor
{
	void RenderSceneObjectsPanel(T_World* world, SceneObjectsPanelContext* panel)
	{
		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayConfig::viewport_width - 600, 50), ImGuiCond_Appearing);
		ImGui::Begin("Scene objects", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::InputText("Search", &panel->search_text[0], 100);
		ImGui::NewLine();

		// copy search text and lowercase it
		std::string _search_text;
		_search_text.assign(panel->search_text);
		Tolower(&_search_text);
		
		auto chunk_iterator = world->GetChunkIterator();
		while (auto* chunk = chunk_iterator())
		{
			auto entity_iter = chunk->GetIterator();
			while (auto* entity = entity_iter())
			{
				std::string name = entity->name;
				Tolower(&name);

				if (panel->search_text == "" || name.find(_search_text) != std::string::npos)
				{
					if (ImGui::Button(entity->name.c_str(), ImVec2(200, 28)))
					{
						panel->active = false;

						if (entity->name == PlayerName)
							OpenPlayerPanel(Player::Get());
						else
							OpenEntityPanel(entity);
					}
				}
			}
		}

		ImGui::End();
	}
}
