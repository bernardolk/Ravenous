#include "EditorSceneObjectsPanel.h"
#include <imgui.h>
#include "EditorEntityPanel.h"
#include "EditorPanelContexts.h"
#include "EditorPlayerPanel.h"
#include "game/entities/EPlayer.h"
#include "engine/io/display.h"
#include "engine/utils/utils.h"
#include "engine/world/World.h"

namespace Editor
{
	void RenderSceneObjectsPanel(RWorld* world, RSceneObjectsPanelContext* panel)
	{
		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayState::viewport_width - 600, 50), ImGuiCond_Appearing);
		ImGui::Begin("Scene objects", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::InputText("Search", &panel->search_text[0], 100);
		ImGui::NewLine();

		// copy search text and lowercase it
		string _search_text;
		_search_text.assign(panel->search_text);
		Tolower(&_search_text);
		
		auto entity_iterator = world->GetEntityIterator();
		while (auto* entity = entity_iterator())
		{
			string name = entity->name;
			Tolower(&name);

			if (panel->search_text == "" || name.find(_search_text) != std::string::npos)
			{
				if (ImGui::Button(entity->name.c_str(), ImVec2(200, 28)))
				{
					panel->active = false;
					
					OpenEntityPanel(entity);
				}
			}
		}

		ImGui::End();
	}
}