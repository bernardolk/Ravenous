#include "editor_entity_panel.h"

#include <imgui.h>

#include "editor.h"
#include "editor_panel_contexts.h"
#include "tools/editor_tools.h"
#include "engine/io/display.h"
#include "engine/render/shader.h"
#include "engine/world/world.h"

namespace Editor
{
	void RenderEntityPanel(EntityPanelContext* panel, T_World* world)
	{
		auto& entity = panel->entity;
		auto& ed_context = *GetContext();

		u32 action_flags = 0;
		bool track = false;

		ImGui::SetNextWindowPos(ImVec2(GlobalDisplayConfig::viewport_width - 550, 200), ImGuiCond_Appearing);
		ImGui::Begin("Entity Panel", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);
		panel->focused = ImGui::IsWindowFocused();

		ImGui::BeginTabBar("##Entity");

		// ----------------
		// > CONTROLS TAB
		// ----------------
		if (ImGui::BeginTabItem("Controls", nullptr, ImGuiTabItemFlags_None))
		{
			ImGui::Text(("Name: " + entity->name).c_str());
			ImGui::Text(("Id: " + std::to_string(entity->id)).c_str());
			ImGui::Text(("Shader: " + entity->shader->name).c_str());

			// RENAME
			ImGui::NewLine();
			if (!panel->rename_option_active)
			{
				if (ImGui::Button("Rename Entity", ImVec2(120, 18)))
					panel->rename_option_active = true;
			}
			else
			{
				ImGui::InputText("New name", &panel->rename_buffer[0], 100);
				if (ImGui::Button("Apply", ImVec2(64, 18)))
				{
					if (panel->ValidateRenameBufferContents())
					{
						entity->name = panel->rename_buffer;
						panel->EmptyRenameBuffer();
						panel->rename_option_active = false;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(64, 18)))
				{
					panel->rename_option_active = false;
				}
			}

			// HIDE ENTITY
			ImGui::SameLine();
			bool _hide_control = entity->flags & EntityFlags_HiddenEntity;
			if (ImGui::Checkbox("Hide Entity", &_hide_control))
			{
				entity->flags ^= EntityFlags_HiddenEntity;
			}

			// MODEL PROPERTIES
			ImGui::NewLine();
			ImGui::Text("Model properties:");
			ImGui::NewLine();


			// POSITION
			{
				float positions[]{entity->position.x, entity->position.y, entity->position.z};
				if (ImGui::DragFloat3("Position", positions, 0.1))
				{
					action_flags |= EntityPanelTA_Position;
					entity->position = vec3{positions[0], positions[1], positions[2]};
				}
				track = track || ImGui::IsItemDeactivatedAfterEdit();
			}

			// ROTATION
			{
				float rotations[]{entity->rotation.x, entity->rotation.y, entity->rotation.z};
				if (ImGui::DragFloat3("Rotation", rotations, 1, -360, 360))
				{
					action_flags |= EntityPanelTA_Rotation;
					entity->rotation = vec3{rotations[0], rotations[1], rotations[2]};
				}
				track = track || ImGui::IsItemDeactivatedAfterEdit();
			}

			// SCALE
			{
				float scaling[]{entity->scale.x, entity->scale.y, entity->scale.z};
				if (ImGui::DragFloat3("Scale", scaling, 0.05, 0, MaxFloat, nullptr))
					action_flags |= EntityPanelTA_Scale;

				track = track || ImGui::IsItemDeactivatedAfterEdit();

				ImGui::SameLine();
				ImGui::Checkbox("Reverse", &panel->reverse_scale);

				if (action_flags & EntityPanelTA_Scale)
				{
					if (panel->reverse_scale)
					{
						auto rot_matrix = entity->GetRotationMatrix();

						if (scaling[0] != entity->scale.x)
							entity->position -= ToVec3(rot_matrix * vec4(scaling[0] - entity->scale.x, 0.f, 0.f, 1.f));
						if (scaling[1] != entity->scale.y)
							entity->position -= ToVec3(rot_matrix * vec4(0.f, scaling[1] - entity->scale.y, 0.f, 1.f));
						if (scaling[2] != entity->scale.z)
							entity->position -= ToVec3(rot_matrix * vec4(0.f, 0.f, scaling[2] - entity->scale.z, 1.f));
					}

					entity->scale = vec3{scaling[0], scaling[1], scaling[2]};
				}
			}

			ImGui::NewLine();

			if (ImGui::Button("Place", ImVec2(82, 18)))
			{
				ActivatePlaceMode(entity);
			}


			ImGui::NewLine();


			// SLIDE INDICATOR
			// if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
			// {
			//   std::string slide_type;
			//    auto inclination = entity->collision_geometry.slope.inclination;
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
				ActivateSnapMode(entity);
			}

			ImGui::SameLine();
			if (ImGui::Checkbox("inside", &ed_context.snap_inside))
			{
				if (ed_context.snap_reference != nullptr)
					SnapEntityToReference(panel->entity);
			}

			if (ImGui::Button("Stretch", ImVec2(82, 18)))
			{
				ActivateStretchMode(entity);
			}

			ImGui::NewLine();
			ImGui::NewLine();

			if (ImGui::CollapsingHeader("World cells"))
			{
				for (auto* chunk : entity->world_chunks)
				{
					ImGui::Text(chunk->GetChunkPositionString().c_str());
				}
			}

			// SHOW GEOMETRIC PROPERTIES
			ImGui::Text("Show:");
			ImGui::Checkbox("Normals", &panel->show_normals);
			ImGui::SameLine();
			ImGui::Checkbox("Collider", &panel->show_collider);
			ImGui::SameLine();
			ImGui::Checkbox("Bounding box", &panel->show_bounding_box);

			// ENTITY INSTANCE CONTROLS
			{
				ImGui::NewLine();
				ImGui::NewLine();
				if (ImGui::Button("Duplicate", ImVec2(82, 18)))
				{
					action_flags |= EntityPanelTA_Duplicate;
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
					action_flags |= EntityPanelTA_Delete;
					ed_context.entity_panel.active = false;
					EditorEraseEntity(entity);
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
			bool is_static = entity->type == EntityType_Static;
			if (ImGui::RadioButton("Static", is_static))
			{
				EM->SetType(entity, EntityType_Static);
			}

			// EntityType_Checkpoint
			bool is_checkpoint = entity->type == EntityType_Checkpoint;
			if (ImGui::RadioButton("Checkpoint", is_checkpoint))
			{
				EM->SetType(entity, EntityType_Checkpoint);
			}

			// EntityType_TimerTrigger
			bool is_timer_trigger = entity->type == EntityType_TimerTrigger;
			if (ImGui::RadioButton("Timer Trigger", is_timer_trigger))
			{
				EM->SetType(entity, EntityType_TimerTrigger);
			}

			// EntityType_TimerTarget
			bool is_timer_target = entity->type == EntityType_TimerTarget;
			if (ImGui::RadioButton("Timer Target", is_timer_target))
			{
				EM->SetType(entity, EntityType_TimerTarget);
			}

			// EntityType_TimerMarking
			bool is_timer_marking = entity->type == EntityType_TimerMarking;
			if (ImGui::RadioButton("Timer Marking", is_timer_marking))
			{
				EM->SetType(entity, EntityType_TimerMarking);
			}

			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::Text("Collider properties");
			ImGui::NewLine();

			ImGui::Checkbox("Slidable", &entity->slidable);

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

				bool a = ImGui::SliderFloat("radius", &entity->trigger_scale.x, 0, 10);
				bool b = ImGui::SliderFloat("height", &entity->trigger_scale.y, 0, 10);

				if (a || b)
					entity->Update();
			}

			else if (is_timer_trigger)
			{
				ImGui::Text("Event trigger");

				bool a = ImGui::SliderFloat("radius", &entity->trigger_scale.x, 0, 10);
				bool b = ImGui::SliderFloat("height", &entity->trigger_scale.y, 0, 10);

				if (a || b)
					entity->Update();

				ImGui::NewLine();

				ImGui::SliderInt("Duration", &entity->timer_trigger_data.timer_duration, 0, 100);

				if (entity->timer_trigger_data.timer_target != nullptr)
				{
					//@todo should be any kind of time_attack_door, but ok
					if (entity->timer_trigger_data.timer_target->timer_target_data.timer_target_type == EntityTimerTargetType_VerticalSlidingDoor)
					{
						auto data = &entity->timer_trigger_data;
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
				if (entity->timer_trigger_data.timer_target == nullptr)
					target_entity_name = "No target selected.";
				else
					target_entity_name = entity->timer_trigger_data.timer_target->name;
				ImGui::Text(target_entity_name.c_str());


				ImGui::SameLine();
				if (ImGui::Button("Show", ImVec2(68, 18)))
				{
					if (entity->timer_trigger_data.timer_target != nullptr)
					{
						panel->show_related_entity = true;
						panel->related_entity = entity->timer_trigger_data.timer_target;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Change", ImVec2(92, 18)))
				{
					panel->show_related_entity = false;
					ActivateSelectEntityAuxTool(&entity->timer_trigger_data.timer_target);
				}
			}

			ImGui::EndTabItem();
		}

		// -------------------
		// > TIMER TARGET TAB
		// -------------------
		if (entity->type == EntityType_TimerTarget)
		{
			if (ImGui::BeginTabItem("Timer Target Settings", nullptr, ImGuiTabItemFlags_None))
			{
				// EntityTimerTargetType_VerticalSlidingDoor
				bool is_vsd = entity->timer_target_data.timer_target_type == EntityTimerTargetType_VerticalSlidingDoor;
				if (ImGui::RadioButton("Vertical Sliding Door", is_vsd))
				{
					entity->timer_target_data.timer_target_type = EntityTimerTargetType_VerticalSlidingDoor;
				}

				ImGui::EndTabItem();
			}
		}
		*/

		// ----------------
		// > TEXTURES TAB
		// ----------------
		if (ImGui::BeginTabItem("Textures", nullptr, ImGuiTabItemFlags_None))
		{
			for (const auto& texture : TextureCatalogue)
			{
				bool in_use = entity->textures[0].name == texture.second.name;
				if (ImGui::RadioButton(texture.second.name.c_str(), in_use))
				{
					entity->textures[0] = texture.second;
				}
			}

			bool _tiled_texture = entity->flags & EntityFlags_RenderTiledTexture;
			if (ImGui::Checkbox("Tiled texture", &_tiled_texture))
			{
				entity->flags ^= EntityFlags_RenderTiledTexture;
				if (_tiled_texture)
					entity->shader = ShaderCatalogue.find("tiledTextureModel")->second;
				else
					entity->shader = ShaderCatalogue.find("model")->second;
			}

			if (entity->flags & EntityFlags_RenderTiledTexture)
			{
				ImGui::Text("Number of tiles for each face:");
				ImGui::SliderInt("Top face", &entity->uv_tile_wrap[0], 0, 15);
				ImGui::SliderInt("Bottom face", &entity->uv_tile_wrap[1], 0, 15);
				ImGui::SliderInt("Front face", &entity->uv_tile_wrap[2], 0, 15);
				ImGui::SliderInt("Left face", &entity->uv_tile_wrap[3], 0, 15);
				ImGui::SliderInt("Right face", &entity->uv_tile_wrap[4], 0, 15);
				ImGui::SliderInt("Back face", &entity->uv_tile_wrap[5], 0, 15);
			}

			ImGui::EndTabItem();
		}

		// ---------------
		// > SHADERS TAB
		// ---------------
		if (ImGui::BeginTabItem("Shaders", nullptr, ImGuiTabItemFlags_None))
		{
			for (const auto& shader : ShaderCatalogue)
			{
				bool in_use = entity->shader->name == shader.second->name;
				if (ImGui::RadioButton(shader.second->name.c_str(), in_use))
				{
					entity->shader = shader.second;
				}
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();

		ImGui::End();

		if (action_flags > 0)
			EntityPanelUpdateEntityAndEditorContext(panel, action_flags, world);
		if (track)
			EntityPanelTrackEntityChanges(panel);
	}


	void EntityPanelTrackEntityChanges(EntityPanelContext* panel)
	{
		auto& ed_context = *GetContext();

		// the following block makes sure that we track the entity original state if necessary.
		// if we already tracked it or we used an external tool from the panel, like grab/move tool, 
		// we don't track, since these tools have their own tracking calls.
		if (!panel->tracked_once)
		{
			EntityState last_recorded_state = ed_context.undo_stack.Check();
			if (last_recorded_state.entity == nullptr || last_recorded_state.entity->id != panel->entity->id)
				ed_context.undo_stack.Track(panel->entity_starting_state);
			panel->tracked_once = true;
		}

		ed_context.undo_stack.Track(panel->entity);
	}


	void EntityPanelUpdateEntityAndEditorContext(const EntityPanelContext* panel, u32 action, T_World* world)
	{
		auto& ed_context = *GetContext();

		if (!(action & EntityPanelTA_Duplicate || action & EntityPanelTA_Delete))
			DeactivateEditorModes();

		panel->entity->Update();
		auto update_cells = world->UpdateEntityWorldCells(panel->entity);
		if (update_cells.status != CellUpdate_OK)
			Rvn::rm_buffer->Add(update_cells.message, 3500);

		// world->UpdateCellsInUseList();

		// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
		UpdateEntityControlArrows(&ed_context.entity_panel);
		UpdateEntityRotationGizmo(&ed_context.entity_panel);
	}


	void OpenEntityPanel(E_Entity* entity)
	{
		auto& ed_context = *GetContext();
		ed_context.selected_entity = entity;

		auto& panel = ed_context.entity_panel;
		panel.active = true;
		panel.entity = entity;
		panel.reverse_scale_x = false;
		panel.reverse_scale_y = false;
		panel.reverse_scale_z = false;
		panel.show_normals = false;
		panel.show_collider = false;
		panel.show_bounding_box = false;
		panel.rename_option_active = false;
		panel.tracked_once = false;
		panel.show_related_entity = false;
		panel.related_entity = nullptr;
		panel.entity_starting_state = GetEntityState(entity);
		panel.EmptyRenameBuffer();

		// TODO: We should _know_ when entities move and be able to act programatically upon that knowledge instead of randomly checking everywhere.
		UpdateEntityControlArrows(&panel);
		UpdateEntityRotationGizmo(&panel);
	}
}
