#include "editor_palette_panel.h"
#include <imgui.h>
#include <stb_image/stb_image.h>
#include "editor.h"
#include "editor_panel_contexts.h"
#include "tools/editor_tools.h"
#include "engine/camera/camera.h"
#include "engine/io/loaders.h"
#include "engine/world/scene_manager.h"
#include "engine/world/world.h"

namespace Editor
{
	void RenderPalettePanel(PalettePanelContext* panel)
	{
		ImGui::SetNextWindowPos(ImVec2(50, 300), ImGuiCond_Always);
		ImGui::Begin("Palette", &panel->active, ImGuiWindowFlags_NoResize);
		ImGui::SetWindowSize("Palette", ImVec2(90, 500), ImGuiCond_Always);
		auto* GSI = GlobalSceneInfo::Get();

		for (unsigned int i = 0; i < panel->count; i++)
		{
			if (ImGui::ImageButton((void*) panel->textures[i], ImVec2(64, 64)))
			{
				auto* new_entity = T_World::Get()->CreateEntity<E_Entity>(0,0,0);
				if (!new_entity)
					continue;
				
				auto attributes = panel->entity_palette[i];

				SetEntityAssets(new_entity, attributes);
				new_entity->position = GSI->camera->position + (2.f * new_entity->scale + 5.f) * GSI->camera->front;
				ActivateMoveMode(new_entity);
			}
		}

		ImGui::End();
	}


	void InitializePalette(PalettePanelContext* panel)
	{
		int texture_count = 0;

		stbi_set_flip_vertically_on_load(false);
		panel->textures[texture_count++] = LoadTextureFromFile("box.png", EditorAssets);
		panel->textures[texture_count++] = LoadTextureFromFile("slope.png", EditorAssets);
		panel->textures[texture_count++] = LoadTextureFromFile("checkpoint.png", EditorAssets);
		stbi_set_flip_vertically_on_load(true);

		// 0
		panel->entity_palette[panel->count++] = EntityAttributes{
		.name = "NONAME",
		.mesh = "aabb",
		.shader = "model",
		.texture = "grey",
		.collision_mesh = "aabb",
		};

		// 1
		panel->entity_palette[panel->count++] = EntityAttributes{
		.name = "NONAME",
		.mesh = "slope",
		.shader = "model",
		.texture = "grey",
		.collision_mesh = "slope",
		};

		// 3
		panel->entity_palette[panel->count++] = EntityAttributes{
		.name = "NONAME-CHECKPOINT",
		.mesh = "aabb",
		.shader = "model",
		.texture = "grey",
		.collision_mesh = "aabb",
		.scale = vec3(0.3, 1.2, 0.3)
		};
	}
}
