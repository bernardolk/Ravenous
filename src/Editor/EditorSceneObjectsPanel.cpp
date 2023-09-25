#include "EditorSceneObjectsPanel.h"
#include <imgui.h>
#include "EditorEntityPanel.h"
#include "EditorPanelContexts.h"
#include "engine/io/display.h"
#include "engine/utils/utils.h"
#include "engine/world/World.h"

namespace Editor
{
	void RenderSceneObjectsPanel(RWorld* World, RSceneObjectsPanelContext* Panel)
	{
		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayState::ViewportWidth - 600, 50), ImGuiCond_Appearing);
		ImGui::Begin("Scene objects", &Panel->Active, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::InputText("Search", &Panel->SearchText[0], 100);
		ImGui::NewLine();

		// copy search text and lowercase it
		string SearchText;
		SearchText.assign(Panel->SearchText);
		Tolower(&SearchText);

		auto EntityIterator = World->GetEntityIterator();
		while (auto* Entity = EntityIterator())
		{
			string Name = Entity->Name;
			Tolower(&Name);

			if (Panel->SearchText == "" || Name.find(SearchText) != std::string::npos)
			{
				if (ImGui::Button(Entity->Name.c_str(), ImVec2(200, 28)))
				{
					Panel->Active = false;

					OpenEntityPanel(Entity);
				}
			}
		}

		ImGui::End();
	}
}
