#include "editor_toolbar.h"
#include <imgui.h>
#include "editor.h"
#include "tools/editor_tools.h"
#include "engine/camera/camera.h"
#include "engine/io/display.h"
#include "engine/serialization/sr_config.h"
#include "engine/world/world.h"

namespace Editor
{
	void RenderToolbar(World* world)
	{
		auto& ed_context = *GetContext();

		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayConfig::viewport_width - 230, 180), ImGuiCond_Appearing);
		ImGui::Begin("Tools", &ed_context.toolbar_active, ImGuiWindowFlags_AlwaysAutoResize);

		string scene_name = "Scene name: " + World::Get()->scene_name;
		ImGui::Text(scene_name.c_str());
		ImGui::NewLine();

		ImGui::InputFloat("##timestep", &Rvn::frame.time_step, 0.5, 1.0, "Timestep = %.1f x");

		ImGui::NewLine();

		// GLOBAL CONFIGS
		{
			bool track = false;

			ImGui::Text("Cam speed");
			ImGui::DragFloat("##camspeed", &CameraManager::Get()->GetCurrentCamera()->acceleration, 0.1, 0.2, MaxFloat);
			track = track || ImGui::IsItemDeactivatedAfterEdit();

			// Ambient light control
			ImGui::Text("Ambient light");
			auto ambient = world->ambient_light;
			float colors[3] = {ambient.x, ambient.y, ambient.z};
			if (ImGui::ColorEdit3("##ambient-color", colors))
			{
				world->ambient_light = vec3{colors[0], colors[1], colors[2]};
			}
			track = track || ImGui::IsItemDeactivatedAfterEdit();

			ImGui::SliderFloat("##ambient-intensity", &world->ambient_intensity, 0, 1, "intensity = %.2f");
			track = track || ImGui::IsItemDeactivatedAfterEdit();

			// save to file changes in config variables
			if (track)
			{
				auto& program_config = *ProgramConfig::Get();
				program_config.camspeed = CameraManager::Get()->GetCurrentCamera()->acceleration;
				program_config.ambient_intensity = world->ambient_intensity;
				program_config.ambient_light = world->ambient_light;
				ConfigSerializer::Save(program_config);
			}

			ImGui::NewLine();
		}

		// PANELS
		if (ImGui::Button("Scene objects", ImVec2(150, 18)))
		{
			ed_context.scene_objects_panel.active = true;
		}

		if (ImGui::Button("Entity Palette", ImVec2(150, 18)))
		{
			ed_context.palette_panel.active = true;
		}

		if (ImGui::Button("World Panel", ImVec2(150, 18)))
		{
			ed_context.world_panel.active = true;
			ed_context.show_world_cells = true;
		}

		if (ImGui::Button("Lights Panel", ImVec2(150, 18)))
		{
			ed_context.lights_panel.active = true;
			ed_context.show_lightbulbs = true;
		}

		if (ImGui::Button("Input Recorder", ImVec2(150, 18)))
		{
			ed_context.input_recorder_panel.active = true;
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
		ImGui::Checkbox("Show Event Triggers", &ed_context.show_event_triggers);
		ImGui::Checkbox("Show World Cells", &ed_context.show_world_cells);
		ImGui::Checkbox("Show Point Lights", &ed_context.show_lightbulbs);
		ImGui::NewLine();
		if (ImGui::Button("Unhide entities"))
		{
			UnhideEntities(world);
		}

		// OPTIONS
		if (ImGui::Button("Collision Logger", ImVec2(150, 18)))
		{
			ed_context.collision_log_panel.active = true;
		}

		// DEBUG OPTIONS
		ImGui::Text("Debug options");
		ImGui::Checkbox("Ledge detection", &ed_context.debug_ledge_detection);

		ImGui::End();
	}
}
