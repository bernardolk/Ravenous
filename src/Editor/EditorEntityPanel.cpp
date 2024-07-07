#include "EditorEntityPanel.h"

#include <imgui.h>

#include "EditorMain.h"
#include "EditorPanelContexts.h"
#include "tools/EditorTools.h"
#include "engine/io/display.h"
#include "engine/render/Shader.h"
#include "engine/world/World.h"
#include "Engine/Entities/Entity.h"
#include "engine/io/loaders.h"

namespace Editor
{
	void RenderEntityPanel(REntityPanelContext* Panel, RWorld* World)
	{
		auto& EdContext = *GetContext();
		
		if (!Panel->Entity.IsValid()) {
			Panel->Active = false;
			return;
		}
		
		auto* Entity = *Panel->Entity;

		uint ActionFlags = 0;
		bool Track = false;

		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayState::ViewportWidth - 550, 200), ImGuiCond_Appearing);
		ImGui::Begin("Entity Panel", &Panel->Active, ImGuiWindowFlags_AlwaysAutoResize);
		Panel->Focused = ImGui::IsWindowFocused();

		ImGui::BeginTabBar("##Entity");

		// ----------------
		// > CONTROLS TAB
		// ----------------
		if (ImGui::BeginTabItem("Controls", nullptr, ImGuiTabItemFlags_None))
		{
			ImGui::Text(("Name: " + Entity->Name).c_str());
			ImGui::Text(("Id: " + std::to_string(Entity->ID)).c_str());
			ImGui::Text(("Shader: " + Entity->Shader->Name).c_str());

			// RENAME
			ImGui::NewLine();
			if (!Panel->RenameOptionActive)
			{
				if (ImGui::Button("Rename Entity", ImVec2(120, 18)))
					Panel->RenameOptionActive = true;
			}
			else
			{
				ImGui::InputText("New name", &Panel->RenameBuffer[0], 100);
				if (ImGui::Button("Apply", ImVec2(64, 18)))
				{
					if (Panel->ValidateRenameBufferContents())
					{
						Entity->Name = Panel->RenameBuffer;
						Panel->EmptyRenameBuffer();
						Panel->RenameOptionActive = false;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(64, 18)))
				{
					Panel->RenameOptionActive = false;
				}
			}

			// HIDE ENTITY
			ImGui::SameLine();
			bool HideControl = Entity->Flags & EntityFlags_HiddenEntity;
			if (ImGui::Checkbox("Hide Entity", &HideControl))
			{
				Entity->Flags ^= EntityFlags_HiddenEntity;
			}

			// MODEL PROPERTIES
			ImGui::NewLine();
			ImGui::Text("Model properties:");
			ImGui::NewLine();


			// POSITION
			{
				float Positions[]{Entity->Position.x, Entity->Position.y, Entity->Position.z};
				if (ImGui::DragFloat3("Position", Positions, 0.1f))
				{
					ActionFlags |= EntityPanelTA_Position;
					Entity->Position = vec3{Positions[0], Positions[1], Positions[2]};
				}
				Track = Track || ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsItemActivated();
			}

			// ROTATION
			{
				float Rotations[]{Entity->Rotation.x, Entity->Rotation.y, Entity->Rotation.z};
				if (ImGui::DragFloat3("Rotation", Rotations, 1, -360, 360))
				{
					ActionFlags |= EntityPanelTA_Rotation;
					Entity->Rotation = vec3{Rotations[0], Rotations[1], Rotations[2]};
				}
				Track = Track || ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsItemActivated();
			}

			// SCALE
			{
				float Scaling[]{Entity->Scale.x, Entity->Scale.y, Entity->Scale.z};
				if (ImGui::DragFloat3("Scale", Scaling, 0.05f, 0, MaxFloat, nullptr))
					ActionFlags |= EntityPanelTA_Scale;

				Track = Track || ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsItemActivated();

				ImGui::SameLine();
				ImGui::Checkbox("Reverse", &Panel->ReverseScale);

				if (ActionFlags & EntityPanelTA_Scale)
				{
					if (Panel->ReverseScale)
					{
						auto RotMatrix = Entity->GetRotationMatrix();
						if (!IsEqual(Scaling[0], Entity->Scale.x)) {
							Entity->Position -= ToVec3(RotMatrix * vec4(Scaling[0] - Entity->Scale.x, 0.f, 0.f, 1.f));
						}
						if (!IsEqual(Scaling[1], Entity->Scale.y)) {
							Entity->Position -= ToVec3(RotMatrix * vec4(0.f, Scaling[1] - Entity->Scale.y, 0.f, 1.f));
						}
						if (!IsEqual(Scaling[2], Entity->Scale.z)) {
							Entity->Position -= ToVec3(RotMatrix * vec4(0.f, 0.f, Scaling[2] - Entity->Scale.z, 1.f));
						}
					}

					Entity->Scale = vec3{Scaling[0], Scaling[1], Scaling[2]};
				}
			}

			ImGui::NewLine();

			if (ImGui::Button("Place", ImVec2(82, 18)))
			{
				ActivatePlaceMode(Entity);
			}

			ImGui::SameLine();

			if (ImGui::Button("Build AABB Collision Mesh", ImVec2(82, 18)))
			{
				// Clear collider geometry
				Entity->Collider.Vertices.clear();
				Entity->Collider.Indices.clear();

				// Create new collision mesh and copy entity mesh vertices into it
				auto* NewCollisionMesh = new RCollisionMesh;
				NewCollisionMesh->Name = Entity->Name + "_GeneratedCMesh";
				Entity->CollisionMesh = NewCollisionMesh;
				for (RVertex& Vertex : Entity->Mesh->Vertices) {
					NewCollisionMesh->Vertices.push_back(Vertex.Position);
				}

				auto BoundingBox = NewCollisionMesh->ComputeBoundingBox();
				NewCollisionMesh->Vertices.clear();
				
				for (auto& Position : BoundingBox.GetVertexPositions()) {
					NewCollisionMesh->Vertices.push_back({Position});
				}
				if (auto** AABB = Find(CollisionGeometryCatalogue, "aabb")) {
					NewCollisionMesh->Indices = (*AABB)->Indices;
				}

				// Updates collider based on new collision mesh.
				Entity->UpdateCollider();

				ExportWavefrontCollisionMesh(Entity->CollisionMesh);
			}

			ImGui::NewLine();

			// ===============================
			// Entity Positioning Tools
			// ===============================

			// Snap
			if (ImGui::Button("Snap", ImVec2(82, 18))) {
				ActivateSnapMode(Entity);
			}

			ImGui::SameLine();
			
			if (ImGui::Checkbox("inside", &EdContext.SnapInside)) {
				if (EdContext.SnapReference.IsValid()) {
					SnapEntityToReference(*Panel->Entity);
				}
			}

			// Stretch
			if (ImGui::Button("Stretch", ImVec2(82, 18))) {
				ActivateStretchMode(Entity);
			}

			ImGui::NewLine();
			ImGui::NewLine();

			// World Cells
			if (ImGui::CollapsingHeader("World cells")) {
				for (auto* Chunk : Entity->WorldChunks) {
					ImGui::Text(Chunk->GetChunkPositionString().c_str());
				}
			}

			// SHOW GEOMETRIC PROPERTIES
			ImGui::Text("Show:");
			ImGui::Checkbox("Bounding box", &Panel->ShowBoundingBox);

			// ===============================
			// Entity Instance Controls
			// ===============================
			{
				ImGui::NewLine();
				ImGui::NewLine();
				if (ImGui::Button("Duplicate", ImVec2(82, 18)))
				{
					ActionFlags |= EntityPanelTA_Duplicate;
					auto* NewEntity = CopyEntity(Entity);
					OpenEntityPanel(NewEntity);
				}

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.6f, 0.6f)));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.7f, 0.7f)));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.8f, 0.8f)));
				if (ImGui::Button("Delete", ImVec2(82, 18)))
				{
					ActionFlags |= EntityPanelTA_Delete;
					EdContext.EntityPanel.Active = false;
					EditorDeleteEntity(Panel->Entity);
				}
				ImGui::PopStyleColor(3);
			}

			ImGui::EndTabItem();
		}

		// ===========================
		//	Model Tab
		// ===========================
		if (ImGui::BeginTabItem("Model", nullptr, ImGuiTabItemFlags_None))
		{
			for (const auto& [Key, Model] : GeometryCatalogue)
			{
				bool bInUse = Entity->Mesh->Name == Model->Name;
				if (ImGui::RadioButton(Model->Name.c_str(), bInUse))
				{
					Entity->Mesh = Model;
				}
			}

			ImGui::EndTabItem();
		}

		// ===========================
		//	Textures Tab
		// ===========================
		if (ImGui::BeginTabItem("Textures", nullptr, ImGuiTabItemFlags_None))
		{
			for (const auto& [TextureName, Texture] : TextureCatalogue)
			{
				bool bInUse = Entity->TextureDiffuse.Name == TextureName;
				if (ImGui::RadioButton(TextureName.c_str(), bInUse))
				{
					Entity->TextureDiffuse = Texture;
				}
			}

			ImGui::EndTabItem();
		}

		// ===========================
		//	Shaders Tab
		// ===========================
		if (ImGui::BeginTabItem("Shaders", nullptr, ImGuiTabItemFlags_None))
		{
			for (const auto& Shader : ShaderCatalogue)
			{
				bool bInUse = Entity->Shader->Name == Shader.second->Name;
				if (ImGui::RadioButton(Shader.second->Name.c_str(), bInUse))
				{
					Entity->Shader = Shader.second;
				}
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();

		ImGui::End();

		// Check if needs to update entity based on changes performed in panel
		if (ActionFlags > 0)
		{
			if (!(ActionFlags & EntityPanelTA_Duplicate || ActionFlags & EntityPanelTA_Delete)) {
				DeactivateEditorModes();
			}

			Panel->Entity->Update();
			auto UpdateCells = World->UpdateEntityWorldChunk(*Panel->Entity);
			if (UpdateCells.Status != CellUpdate_OK) {
				// PrintEditorMsg(UpdateCells.Message);
			}
			
			EdContext.bGizmoPositionsDirty = true;
		}

		// Checks if needs to track entity state change
		if (Track) {
			EdContext.UndoStack.TrackTransformChange(Panel->Entity);
		}
	}

	void OpenEntityPanel(EEntity* Entity)
	{
		auto& EdContext = *GetContext();
		EdContext.SelectedEntity = MakeHandle<EEntity>(Entity);

		auto& Panel = EdContext.EntityPanel;
		Panel.Active= true;
		Panel.Entity = EdContext.SelectedEntity;
		Panel.ReverseScaleX= false;
		Panel.ReverseScaleY= false;
		Panel.ReverseScaleZ= false;
		Panel.ShowBoundingBox= false;
		Panel.RenameOptionActive= false;
		Panel.TrackedOnce= false;
		Panel.ShowRelatedEntity= false;
		Panel.RelatedEntity = {};
		Panel.EntityStartingState = GetEntityState(Panel.Entity);
		Panel.EmptyRenameBuffer();

		EdContext.bGizmoPositionsDirty = true;
	}
}
