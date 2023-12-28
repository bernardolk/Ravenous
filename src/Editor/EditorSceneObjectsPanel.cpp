#include "EditorSceneObjectsPanel.h"
#include <imgui.h>
#include "EditorEntityPanel.h"
#include "EditorPanelContexts.h"
#include "Engine/Io/Display.h"
#include "Engine/Utils/Utils.h"
#include "Engine/World/World.h"
#include "Engine/Entities/Entity.h"

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
