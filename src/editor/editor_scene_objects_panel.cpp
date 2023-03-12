#include "editor_scene_objects_panel.h"
#include <imgui.h>
#include "editor_entity_panel.h"
#include "editor_panel_contexts.h"
#include "editor_player_panel.h"
#include "game/entities/player.h"
#include "engine/io/display.h"
#include "engine/world/world.h"
#include "engine/utils/utils.h"

namespace Editor
{
	void render_scene_objects_panel(const World* world, SceneObjectsPanelContext* panel)
	{
		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayConfig::viewport_width - 600, 50), ImGuiCond_Appearing);
		ImGui::Begin("Scene objects", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::InputText("Search", &panel->search_text[0], 100);
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

			if (panel->search_text == "" || name.find(_search_text) != std::string::npos)
			{
				if (ImGui::Button(entity->name.c_str(), ImVec2(200, 28)))
				{
					panel->active = false;

					if (entity->name == PlayerName)
						open_player_panel(Player::Get());
					else
						open_entity_panel(entity);
				}
			}
		}

		ImGui::End();
	}
}
