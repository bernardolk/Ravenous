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
		auto& Entity = Panel->Entity;
		auto& EdContext = *GetContext();

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
				if (ImGui::DragFloat3("Position", Positions, 0.1))
				{
					ActionFlags |= EntityPanelTA_Position;
					Entity->Position = vec3{Positions[0], Positions[1], Positions[2]};
				}
				Track = Track || ImGui::IsItemDeactivatedAfterEdit();
			}

			// ROTATION
			{
				float Rotations[]{Entity->Rotation.x, Entity->Rotation.y, Entity->Rotation.z};
				if (ImGui::DragFloat3("Rotation", Rotations, 1, -360, 360))
				{
					ActionFlags |= EntityPanelTA_Rotation;
					Entity->Rotation = vec3{Rotations[0], Rotations[1], Rotations[2]};
				}
				Track = Track || ImGui::IsItemDeactivatedAfterEdit();
			}

			// SCALE
			{
				float Scaling[]{Entity->Scale.x, Entity->Scale.y, Entity->Scale.z};
				if (ImGui::DragFloat3("Scale", Scaling, 0.05, 0, MaxFloat, nullptr))
					ActionFlags |= EntityPanelTA_Scale;

				Track = Track || ImGui::IsItemDeactivatedAfterEdit();

				ImGui::SameLine();
				ImGui::Checkbox("Reverse", &Panel->ReverseScale);

				if (ActionFlags & EntityPanelTA_Scale)
				{
					if (Panel->ReverseScale)
					{
						auto RotMatrix = Entity->GetRotationMatrix();

						if (Scaling[0] != Entity->Scale.x)
							Entity->Position -= ToVec3(RotMatrix * vec4(Scaling[0] - Entity->Scale.x, 0.f, 0.f, 1.f));
						if (Scaling[1] != Entity->Scale.y)
							Entity->Position -= ToVec3(RotMatrix * vec4(0.f, Scaling[1] - Entity->Scale.y, 0.f, 1.f));
						if (Scaling[2] != Entity->Scale.z)
							Entity->Position -= ToVec3(RotMatrix * vec4(0.f, 0.f, Scaling[2] - Entity->Scale.z, 1.f));
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

				// Build a bounding box for the mesh and then set the aabb vertices as collision mesh vertices. Indices should be the same as the aabb collision mesh .obj model so we just copy over.
				// NOTE: The collision mesh version of aabb.obj has significantly less vertices / indices due to the fact that it doesn't need to hold normal / texel information, therefore there's no need to split a corner vertice
				// into multiple RVertex instances. So we can't use the vertex data from a regular, non RCollisionMesh, RMesh aabb for collision mesh purposes.
				auto BoundingBox = NewCollisionMesh->ComputeBoundingBox();
				NewCollisionMesh->Vertices.clear();
				for (auto& Position : BoundingBox.GetVertexPositions()) {
					NewCollisionMesh->Vertices.push_back(Position);
				}
				if (auto** AABB = Find(CollisionGeometryCatalogue, "aabb")) {
					NewCollisionMesh->Indices = (*AABB)->Indices;
				}

				// Updates collider based on new collision mesh.
				Entity->UpdateCollider();

				ExportWavefrontCollisionMesh(Entity->CollisionMesh);
			}

			ImGui::NewLine();


			// SLIDE INDICATOR
			// if(Entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
			// {
			//   std::string slide_type;
			//    auto inclination = Entity->collision_geometry.slope.inclination;
			//    if(inclination > SLIDE_MAX_ANGLE)
			//       slide_type = "Player will: slide fall";
			//    else if(inclination > SLIDE_MIN_ANGLE)
			//       slide_type = "Player will: slide";
			//    else
			//       slide_type = "Player will: stand";

			//    ImGui::Text(slide_type.c_str());
			// }

			// ENTITY POSITIONING TOOLS
			if (ImGui::Button("Snap", ImVec2(82, 18)))
			{
				ActivateSnapMode(Entity);
			}

			ImGui::SameLine();
			if (ImGui::Checkbox("inside", &EdContext.SnapInside))
			{
				if (EdContext.SnapReference != nullptr)
					SnapEntityToReference(Panel->Entity);
			}

			if (ImGui::Button("Stretch", ImVec2(82, 18)))
			{
				ActivateStretchMode(Entity);
			}

			ImGui::NewLine();
			ImGui::NewLine();

			if (ImGui::CollapsingHeader("World cells"))
			{
				for (auto* Chunk : Entity->WorldChunks)
				{
					ImGui::Text(Chunk->GetChunkPositionString().c_str());
				}
			}

			// SHOW GEOMETRIC PROPERTIES
			ImGui::Text("Show:");
			ImGui::Checkbox("Normals", &Panel->ShowNormals);
			ImGui::SameLine();
			ImGui::Checkbox("Bounding box", &Panel->ShowBoundingBox);

			// ENTITY INSTANCE CONTROLS
			{
				ImGui::NewLine();
				ImGui::NewLine();
				if (ImGui::Button("Duplicate", ImVec2(82, 18)))
				{
					ActionFlags |= EntityPanelTA_Duplicate;
					// TODO: reimplement
					// auto new_entity = EM->CopyEntity(entity);
					// OpenEntityPanel(new_entity);
				}

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.6f, 0.6f)));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.7f, 0.7f)));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(0.03f, 0.8f, 0.8f)));
				if (ImGui::Button("Delete", ImVec2(82, 18)))
				{
					ActionFlags |= EntityPanelTA_Delete;
					EdContext.EntityPanel.Active = false;
					EditorEraseEntity(Entity);
				}
				ImGui::PopStyleColor(3);
			}

			ImGui::EndTabItem();
		}


		// -------------------
		// > ENTITY TYPE TAB
		// -------------------
		/**
		if (ImGui::BeginTabItem("Attributes", nullptr, ImGuiTabItemFlags_None))
		{
			ImGui::Text("Entity Type");
			
			// EntityType_Static
			bool is_static = Entity->type == EntityType_Static;
			if (ImGui::RadioButton("Static", is_static))
			{
				EM->SetType(entity, EntityType_Static);
			}

			// EntityType_Checkpoint
			bool is_checkpoint = Entity->type == EntityType_Checkpoint;
			if (ImGui::RadioButton("Checkpoint", is_checkpoint))
			{
				EM->SetType(entity, EntityType_Checkpoint);
			}

			// EntityType_TimerTrigger
			bool is_timer_trigger = Entity->type == EntityType_TimerTrigger;
			if (ImGui::RadioButton("Timer Trigger", is_timer_trigger))
			{
				EM->SetType(entity, EntityType_TimerTrigger);
			}

			// EntityType_TimerTarget
			bool is_timer_target = Entity->type == EntityType_TimerTarget;
			if (ImGui::RadioButton("Timer Target", is_timer_target))
			{
				EM->SetType(entity, EntityType_TimerTarget);
			}

			// EntityType_TimerMarking
			bool is_timer_marking = Entity->type == EntityType_TimerMarking;
			if (ImGui::RadioButton("Timer Marking", is_timer_marking))
			{
				EM->SetType(entity, EntityType_TimerMarking);
			}

			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::Text("Collider properties");
			ImGui::NewLine();

			ImGui::Checkbox("Slidable", &Entity->slidable);

			ImGui::NewLine();
			ImGui::NewLine();

			if (!is_static)
			{
				ImGui::Text("Entity Type Properties");
				ImGui::NewLine();
			}

			if (is_checkpoint)
			{
				ImGui::Text("Event trigger");

				bool a = ImGui::SliderFloat("radius", &Entity->trigger_scale.x, 0, 10);
				bool b = ImGui::SliderFloat("height", &Entity->trigger_scale.y, 0, 10);

				if (a || b)
					Entity->Update();
			}

			else if (is_timer_trigger)
			{
				ImGui::Text("Event trigger");

				bool a = ImGui::SliderFloat("radius", &Entity->trigger_scale.x, 0, 10);
				bool b = ImGui::SliderFloat("height", &Entity->trigger_scale.y, 0, 10);

				if (a || b)
					Entity->Update();

				ImGui::NewLine();

				ImGui::SliderInt("Duration", &Entity->timer_trigger_data.timer_duration, 0, 100);

				if (Entity->timer_trigger_data.timer_target != nullptr)
				{
					//@todo should be any kind of time_attack_door, but ok
					if (Entity->timer_trigger_data.timer_target->timer_target_data.timer_target_type == EntityTimerTargetType_VerticalSlidingDoor)
					{
						auto data = &Entity->timer_trigger_data;
						int empty_slot = -1;
						bool there_is_at_least_one_marking = false;
						For(data->size)
						{
							if (data->markings[i] != nullptr)
							{
								if (!there_is_at_least_one_marking)
								{
									// renders header for this section
									ImGui::Text("Marking entity     -     Time checkpoint (s)     -     Delete");
									there_is_at_least_one_marking = true;
								}
								ImGui::Button(data->markings[i]->name.c_str(), ImVec2(48, 18));
								ImGui::SameLine();

								std::string dint_id = "##duration-" + std::to_string(i);
								ImGui::DragInt(dint_id.c_str(), (int*)&data->time_checkpoints[i], 1, 0, 10000);
								if (ImGui::Button("Delete", ImVec2(32, 18)))
								{
									data->DeleteMarking(i);
								}
							}
							else if (empty_slot == -1)
							{
								empty_slot = i;
							}
						}

						if (empty_slot >= 0)
						{
							ImGui::NewLine();

							if (ImGui::Button("Add marking", ImVec2(60, 18)))
							{
								panel->show_related_entity = false;

								auto callback_args = EdToolCallbackArgs();
								callback_args.entity = entity;
								callback_args.entity_type = EntityType_TimerMarking;

								ActivateSelectEntityAuxTool(
									&data->markings[empty_slot],
									EdToolCallback_EntityManagerSetType,
									callback_args
								);
							}
						}
						else
						{
							std::string text = "Limit of " + std::to_string(data->size) + " markings reached. Can't add another one";
							ImGui::Text(text.c_str());
						}
					}
				}

				// change timer target
				ImGui::NewLine();

				ImGui::Text("Timer target");
				std::string target_entity_name;
				if (Entity->timer_trigger_data.timer_target == nullptr)
					target_entity_name = "No target selected.";
				else
					target_entity_name = Entity->timer_trigger_data.timer_target->name;
				ImGui::Text(target_entity_name.c_str());


				ImGui::SameLine();
				if (ImGui::Button("Show", ImVec2(68, 18)))
				{
					if (Entity->timer_trigger_data.timer_target != nullptr)
					{
						panel->show_related_entity = true;
						panel->related_entity = Entity->timer_trigger_data.timer_target;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Change", ImVec2(92, 18)))
				{
					panel->show_related_entity = false;
					ActivateSelectEntityAuxTool(&Entity->timer_trigger_data.timer_target);
				}
			}

			ImGui::EndTabItem();
		}

		// -------------------
		// > TIMER TARGET TAB
		// -------------------
		if (Entity->type == EntityType_TimerTarget)
		{
			if (ImGui::BeginTabItem("Timer Target Settings", nullptr, ImGuiTabItemFlags_None))
			{
				// EntityTimerTargetType_VerticalSlidingDoor
				bool is_vsd = Entity->timer_target_data.timer_target_type == EntityTimerTargetType_VerticalSlidingDoor;
				if (ImGui::RadioButton("Vertical Sliding Door", is_vsd))
				{
					Entity->timer_target_data.timer_target_type = EntityTimerTargetType_VerticalSlidingDoor;
				}

				ImGui::EndTabItem();
			}
		}
		*/

		// ---------------
		// > MODEL TAB
		// ---------------
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

		// ----------------
		// > TEXTURES TAB
		// ----------------
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

		// ---------------
		// > SHADERS TAB
		// ---------------
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

		if (ActionFlags > 0)
			EntityPanelUpdateEntityAndEditorContext(Panel, ActionFlags, World);
		if (Track)
			EntityPanelTrackEntityChanges(Panel);
	}


	void EntityPanelTrackEntityChanges(REntityPanelContext* Panel)
	{
		auto& EdContext = *GetContext();

		// the following block makes sure that we track the entity original state if necessary.
		// if we already tracked it or we used an external tool from the panel, like grab/move tool, 
		// we don't track, since these tools have their own tracking calls.
		if (!Panel->TrackedOnce)
		{
			REntityState LastRecordedState = EdContext.UndoStack.Check();
			if (LastRecordedState.Entity == nullptr || LastRecordedState.Entity->ID != Panel->Entity->ID)
				EdContext.UndoStack.Track(Panel->EntityStartingState);
			Panel->TrackedOnce = true;
		}

		EdContext.UndoStack.Track(Panel->Entity);
	}


	void EntityPanelUpdateEntityAndEditorContext(const REntityPanelContext* Panel, uint Action, RWorld* World)
	{
		auto& EdContext = *GetContext();

		if (!(Action & EntityPanelTA_Duplicate || Action & EntityPanelTA_Delete))
			DeactivateEditorModes();

		Panel->Entity->Update();
		auto UpdateCells = World->UpdateEntityWorldChunk(Panel->Entity);
		if (UpdateCells.Status != CellUpdate_OK)
			Rvn::RmBuffer->Add(UpdateCells.Message, 3500);

		// world->UpdateCellsInUseList();

		// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
		UpdateEntityControlArrows(&EdContext.EntityPanel);
		UpdateEntityRotationGizmo(&EdContext.EntityPanel);
	}


	void OpenEntityPanel(EEntity* Entity)
	{
		auto& EdContext = *GetContext();
		EdContext.SelectedEntity = Entity;

		auto& Panel = EdContext.EntityPanel;
		Panel.Active= true;
		Panel.Entity= Entity;
		Panel.ReverseScaleX= false;
		Panel.ReverseScaleY= false;
		Panel.ReverseScaleZ= false;
		Panel.ShowNormals= false;
		Panel.ShowBoundingBox= false;
		Panel.RenameOptionActive= false;
		Panel.TrackedOnce= false;
		Panel.ShowRelatedEntity= false;
		Panel.RelatedEntity= nullptr;
		Panel.EntityStartingState = GetEntityState(Entity);
		Panel.EmptyRenameBuffer();

		// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
		UpdateEntityControlArrows(&Panel);
		UpdateEntityRotationGizmo(&Panel);
	}
}
