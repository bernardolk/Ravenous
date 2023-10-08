#include "EditorToolbar.h"
#include <imgui.h>
#include "EditorMain.h"
#include "Engine/RavenousEngine.h"
#include "tools/EditorTools.h"
#include "engine/camera/camera.h"
#include "engine/io/display.h"
#include "engine/serialization/sr_config.h"
#include "engine/world/World.h"

namespace Editor
{
	void RenderToolbar(RWorld* World)
	{
		auto& EdContext = *GetContext();

		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayState::ViewportWidth - 230, 180), ImGuiCond_Appearing);
		ImGui::Begin("Tools", &EdContext.ToolbarActive, ImGuiWindowFlags_AlwaysAutoResize);

		string SceneName = "Scene name: " + RWorld::Get()->SceneName;
		ImGui::Text(SceneName.c_str());
		ImGui::NewLine();

		ImGui::InputFloat("##timestep", &World->GetFrameData().TimeStep, 0.5, 1.0, "Timestep = %.1f x");

		ImGui::NewLine();

		// GLOBAL CONFIGS
		{
			bool Track = false;

			ImGui::Text("Cam speed");
			ImGui::DragFloat("##camspeed", &RCameraManager::Get()->GetCurrentCamera()->Acceleration, 0.1, 0.2, MaxFloat);
			Track = Track || ImGui::IsItemDeactivatedAfterEdit();

			// Ambient light control
			ImGui::Text("Ambient light");
			auto Ambient = World->AmbientLight;
			float Colors[3] = {Ambient.x, Ambient.y, Ambient.z};
			if (ImGui::ColorEdit3("##ambient-color", Colors))
			{
				World->AmbientLight = vec3{Colors[0], Colors[1], Colors[2]};
			}
			Track = Track || ImGui::IsItemDeactivatedAfterEdit();

			ImGui::SliderFloat("##ambient-intensity", &World->AmbientIntensity, 0, 1, "intensity = %.2f");
			Track = Track || ImGui::IsItemDeactivatedAfterEdit();

			// save to file changes in config variables
			if (Track)
			{
				auto& ProgramConfig = *ProgramConfig::Get();
				ProgramConfig.Camspeed = RCameraManager::Get()->GetCurrentCamera()->Acceleration;
				ProgramConfig.AmbientIntensity = World->AmbientIntensity;
				ProgramConfig.AmbientLight = World->AmbientLight;
				ConfigSerializer::Save(ProgramConfig);
			}

			ImGui::NewLine();
		}

		// PANELS
		if (ImGui::Button("Scene objects", ImVec2(150, 18)))
		{
			EdContext.SceneObjectsPanel.Active = true;
		}

		if (ImGui::Button("Entity Palette", ImVec2(150, 18)))
		{
			EdContext.PalettePanel.Active = true;
		}

		if (ImGui::Button("World Panel", ImVec2(150, 18)))
		{
			EdContext.WorldPanel.Active = true;
			EdContext.ShowWorldCells = true;
		}

		if (ImGui::Button("Lights Panel", ImVec2(150, 18)))
		{
			EdContext.LightsPanel.Active = true;
			EdContext.ShowLightbulbs = true;
		}

		if (ImGui::Button("Input Recorder", ImVec2(150, 18)))
		{
			EdContext.InputRecorderPanel.Active = true;
		}

		ImGui::NewLine();

		// TOOLS
		ImGui::Text("Measure");
		if (ImGui::Button("X", ImVec2(40, 18)))
		{
			ActivateMeasureMode(0);
		}

		ImGui::SameLine();
		if (ImGui::Button("Y", ImVec2(40, 18)))
		{
			ActivateMeasureMode(1);
		}

		ImGui::SameLine();
		if (ImGui::Button("Z", ImVec2(40, 18)))
		{
			ActivateMeasureMode(2);
		}

		ImGui::NewLine();
		if (ImGui::Button("Locate Coordinates", ImVec2(150, 18)))
		{
			ActivateLocateCoordsMode();
		}


		// SHOW STUFF
		ImGui::Checkbox("Show Event Triggers", &EdContext.ShowEventTriggers);
		ImGui::Checkbox("Show World Cells", &EdContext.ShowWorldCells);
		ImGui::Checkbox("Show Point Lights", &EdContext.ShowLightbulbs);
		ImGui::NewLine();
		if (ImGui::Button("Unhide entities"))
		{
			UnhideEntities(World);
		}

		// OPTIONS
		if (ImGui::Button("Collision Logger", ImVec2(150, 18)))
		{
			EdContext.CollisionLogPanel.Active = true;
		}

		// DEBUG OPTIONS
		ImGui::Text("Debug options");
		ImGui::Checkbox("Ledge detection", &EdContext.DebugLedgeDetection);

		ImGui::End();
	}
}
