#include "EditorPalettePanel.h"
#include <imgui.h>
#include <stb_image/stb_image.h>
#include "EditorMain.h"
#include "EditorPanelContexts.h"
#include "tools/EditorTools.h"
#include "engine/camera/camera.h"
#include "engine/entities/StaticMesh.h"
#include "engine/io/loaders.h"
#include "engine/world/World.h"

namespace Editor
{
	void RenderPalettePanel(RPalettePanelContext* Panel)
	{
		ImGui::SetNextWindowPos(ImVec2(50, 300), ImGuiCond_Always);
		ImGui::Begin("Palette", &Panel->Active, ImGuiWindowFlags_NoResize);
		ImGui::SetWindowSize("Palette", ImVec2(90, 500), ImGuiCond_Always);
		auto* CamManager = RCameraManager::Get();
		auto* Camera = CamManager->GetCurrentCamera();

		for (unsigned int I = 0; I < Panel->Count; I++)
		{
			if (ImGui::ImageButton((void*)Panel->Textures[I], ImVec2(64, 64)))
			{
				auto Attributes = Panel->EntityPalette[I];

				EHandle<EStaticMesh> NewEntity = SpawnEntity<EStaticMesh>();

				SetEntityAssets(*NewEntity, Attributes);
				FindNameForNewEntity(*NewEntity);
				
				NewEntity->Position = Camera->Position + (2.f * NewEntity->Scale + 5.f) * Camera->Front;
				ActivateMoveMode(*NewEntity);
				GetContext()->UndoStack.TrackCreation(NewEntity);
			}
		}

		ImGui::End();
	}


	void InitializePalette(RPalettePanelContext* Panel)
	{
		int TextureCount = 0;

		stbi_set_flip_vertically_on_load(false);
		Panel->Textures[TextureCount++] = LoadTextureFromFile("box.png", EditorAssets);
		Panel->Textures[TextureCount++] = LoadTextureFromFile("slope.png", EditorAssets);
		Panel->Textures[TextureCount++] = LoadTextureFromFile("checkpoint.png", EditorAssets);
		stbi_set_flip_vertically_on_load(true);

		// 0
		Panel->EntityPalette[Panel->Count++] = REntityAttributes{
			.Name = "NONAME",
			.Mesh = "aabb",
			.Shader = "model",
			.Texture = "grey",
			.CollisionMesh = "aabb",
		};

		// 1
		Panel->EntityPalette[Panel->Count++] = REntityAttributes{
			.Name = "NONAME",
			.Mesh = "slope",
			.Shader = "model",
			.Texture = "grey",
			.CollisionMesh = "slope",
		};

		// 3
		Panel->EntityPalette[Panel->Count++] = REntityAttributes{
			.Name = "NONAME-CHECKPOINT",
			.Mesh = "aabb",
			.Shader = "model",
			.Texture = "grey",
			.CollisionMesh = "aabb",
			.Scale = vec3(0.3, 1.2, 0.3)
		};
	}
}
