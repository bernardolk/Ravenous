#include "EditorPalettePanel.h"
#include <imgui.h>
#include <stb_image/stb_image.h>
#include "editor.h"
#include "EditorPanelContexts.h"
#include "tools/EditorTools.h"
#include "engine/camera/camera.h"
#include "engine/entities/StaticMesh.h"
#include "engine/io/loaders.h"
#include "engine/world/World.h"

namespace Editor
{
	void RenderPalettePanel(RPalettePanelContext* panel)
	{
		ImGui::SetNextWindowPos(ImVec2(50, 300), ImGuiCond_Always);
		ImGui::Begin("Palette", &panel->active, ImGuiWindowFlags_NoResize);
		ImGui::SetWindowSize("Palette", ImVec2(90, 500), ImGuiCond_Always);
		auto* cam_manager = RCameraManager::Get();
		auto* camera = cam_manager->GetCurrentCamera();

		for (unsigned int i = 0; i < panel->count; i++)
		{
			if (ImGui::ImageButton((void*) panel->textures[i], ImVec2(64, 64)))
			{
				auto* new_entity = RWorld::Get()->SpawnEntity<EStaticMesh>();
				if (!new_entity)
					continue;
				
				auto attributes = panel->entity_palette[i];

				SetEntityAssets(new_entity, attributes);
				new_entity->position = camera->position + (2.f * new_entity->scale + 5.f) * camera->front;
				ActivateMoveMode(new_entity);
			}
		}

		ImGui::End();
	}


	void InitializePalette(RPalettePanelContext* panel)
	{
		int texture_count = 0;

		stbi_set_flip_vertically_on_load(false);
		panel->textures[texture_count++] = LoadTextureFromFile("box.png", EditorAssets);
		panel->textures[texture_count++] = LoadTextureFromFile("slope.png", EditorAssets);
		panel->textures[texture_count++] = LoadTextureFromFile("checkpoint.png", EditorAssets);
		stbi_set_flip_vertically_on_load(true);

		// 0
		panel->entity_palette[panel->count++] = REntityAttributes{
		.name = "NONAME",
		.mesh = "aabb",
		.shader = "model",
		.texture = "grey",
		.collision_mesh = "aabb",
		};

		// 1
		panel->entity_palette[panel->count++] = REntityAttributes{
		.name = "NONAME",
		.mesh = "slope",
		.shader = "model",
		.texture = "grey",
		.collision_mesh = "slope",
		};

		// 3
		panel->entity_palette[panel->count++] = REntityAttributes{
		.name = "NONAME-CHECKPOINT",
		.mesh = "aabb",
		.shader = "model",
		.texture = "grey",
		.collision_mesh = "aabb",
		.scale = vec3(0.3, 1.2, 0.3)
		};
	}
}
