// -------------
// ENTITY PANEL
// -------------
void undo_selected_entity_move_changes();
void open_entity_panel(Entity* entity);
void check_for_asset_changes();
void update_entity_control_arrows(EntityPanelContext* panel);
void render_entity_control_arrows(EntityPanelContext* panel);

void render_entity_panel(EntityPanelContext* panel);

void render_entity_panel(EntityPanelContext* panel)
{
   auto& entity = panel->entity;
   ImGui::SetNextWindowPos(ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH - 550, 370), ImGuiCond_Appearing);

   ImGui::Begin("Entity Panel", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);
   panel->focused = ImGui::IsWindowFocused();

   ImGui::Text(("Name: " + entity->name).c_str());
   ImGui::Text(("Id: " + to_string(entity->id)).c_str());
   ImGui::Text(("Shader: " + entity->shader->name).c_str());

   // entity state tracking
   bool track = false;

   // POSITION
   ImGui::NewLine();
   bool used_pos = false;
   {
      float positions[]{ entity->position.x, entity->position.y, entity->position.z };
      if(ImGui::DragFloat3("Position", positions, 0.1))
      {
         used_pos = true;
         entity->position = vec3{positions[0], positions[1], positions[2]};
      }
      track = track || ImGui::IsItemDeactivatedAfterEdit();
   }

   // ROTATION
   bool used_rot = false;
   {
      float rotations[]{ entity->rotation.x, entity->rotation.y, entity->rotation.z };
      if(ImGui::DragFloat3("Rotation", rotations, 1, 0, 360))
      {
         used_rot = true;
         entity->rotation = vec3{rotations[0], rotations[1], rotations[2]};
      }
      track = track || ImGui::IsItemDeactivatedAfterEdit();
   }

   // SCALE
   bool used_scaling = false;
   {
      float scaling[]{ entity->scale.x, entity->scale.y, entity->scale.z };
      if(ImGui::DragFloat3("Scale", scaling, 0.05, 0, MAX_FLOAT, NULL))
         used_scaling = true;

      track = track || ImGui::IsItemDeactivatedAfterEdit();

      ImGui::SameLine();
      ImGui::Checkbox("Reverse", &panel->reverse_scale);

      if(used_scaling)
      {
         if(panel->reverse_scale)
         {
            auto rot_matrix = entity->get_rotation_matrix();

            if(scaling[0] != entity->scale.x)
               entity->position -= toVec3(rot_matrix * vec4(scaling[0] - entity->scale.x, 0.f, 0.f, 1.f));
            if(scaling[1] != entity->scale.y)
               entity->position -= toVec3(rot_matrix * vec4(0.f, scaling[1] - entity->scale.y, 0.f, 1.f));
            if(scaling[2] != entity->scale.z)
               entity->position -= toVec3(rot_matrix * vec4(0.f, 0.f, scaling[2] - entity->scale.z, 1.f));
         }

         entity->scale = vec3{scaling[0], scaling[1], scaling[2]};
      }
   }


   // SLIDE INDICATOR
   if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
   {
      string slide_type;
      auto inclination = entity->collision_geometry.slope.inclination;
      if(inclination > SLIDE_MAX_ANGLE)
         slide_type = "Player will: slide fall";
      else if(inclination > SLIDE_MIN_ANGLE)
         slide_type = "Player will: slide";
      else
         slide_type = "Player will: stand";

      ImGui::Text(slide_type.c_str());
   }

   // ENTITY POSITIONING TOOLS
   if(ImGui::Button("Snap", ImVec2(82,18)))
   {
      activate_snap_mode(entity);
   }

   ImGui::SameLine();
   if(ImGui::Checkbox("inside", &EdContext.snap_inside))
   {
      if(EdContext.snap_reference != nullptr)
         snap_entity_to_reference(panel->entity);
   }

   ImGui::SameLine();
   if(ImGui::Button("Stretch", ImVec2(82,18)))
   {
      activate_stretch_mode(entity);
   }

   if(ImGui::Button("Place", ImVec2(82,18)))
   {
      activate_place_mode(entity);
   }

   // TABS

   // CHECKPOINT
   if(entity->type == CHECKPOINT)
   {
      ImGui::NewLine();
      ImGui::Text("Event trigger");
      ImGui::SliderFloat("radius", &entity->trigger_scale.x, 0, 10);
      ImGui::SliderFloat("height", &entity->trigger_scale.y, 0, 10);
   }
   
   ImGui::NewLine();
   if(ImGui::CollapsingHeader("World cells"))
   {
      for(int i = 0; i < entity->world_cells_count; i++)
      {
         auto cell = entity->world_cells[i];
         ImGui::Text(cell->coords_str().c_str());
      }   
   }

   if(ImGui::CollapsingHeader("Textures"))
   {
      for(auto const& texture : Texture_Catalogue)
      {
         bool in_use = entity->textures[0].name == texture.second.name;
         if(ImGui::RadioButton(texture.second.name.c_str(), in_use))
         {
            entity->textures[0] = texture.second;
         }
      }
   }

   // RENAME
   ImGui::NewLine();
   if(ImGui::CollapsingHeader("Rename Entity"))
   {
      ImGui::InputText("New name", &panel->rename_buffer[0], 100);
      ImGui::SameLine();
      if(ImGui::Button("Apply"))
      {
         if(panel->validate_rename_buffer_contents())
         {
            entity->name = panel->rename_buffer;
            panel->empty_rename_buffer();
         }
      }
   }

   // HIDE ENTITY
   ImGui::Checkbox("Hide", &entity->wireframe);
   ImGui::SameLine();
   ImGui::Text("Show");
   ImGui::Checkbox("Normals", &panel->show_normals);
   ImGui::SameLine();
   ImGui::Checkbox("Collider", &panel->show_collider);
   ImGui::SameLine();
   ImGui::Checkbox("Bounding box", &panel->show_bounding_box);


   // ENTITY INSTANCE CONTROLS
   bool duplicated = false;
   bool deleted = false;
   {
      ImGui::NewLine();
      ImGui::NewLine();
      if(ImGui::Button("Duplicate", ImVec2(82,18)))
      {
         duplicated = true;
         auto new_entity = Entity_Manager.copy_entity(entity);
         open_entity_panel(new_entity);
      }

      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.03f, 0.6f, 0.6f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.03f, 0.7f, 0.7f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.03f, 0.8f, 0.8f));
      if(ImGui::Button("Delete", ImVec2(82,18)))
      {
         deleted = true;         
         EdContext.entity_panel.active = false;
         editor_erase_entity(entity);
      }
      ImGui::PopStyleColor(3);
   }

   ImGui::NewLine();
   ImGui::NewLine();
   // LAYOUT END

   if(track)
   {
      EdContext.undo_stack.track(entity);
      panel->entity_tracked_state = get_entity_state(entity);
   }

   // ----------------
   // Action happened
   // ----------------
   if(used_pos || used_rot || used_scaling || duplicated || deleted)
   {
      if(!(duplicated || deleted))
         deactivate_editor_modes();

      // needs to be done here to prevent a bug
      entity->old_update_collision_geometry(); 

      auto update_cells = World.update_entity_world_cells(entity);
      if(update_cells.status != CellUpdate_OK)
         G_BUFFERS.rm_buffer->add(update_cells.message, 3500);

      World.update_cells_in_use_list();
   }

   ImGui::End();
}


void open_entity_panel(Entity* entity)
{
   EdContext.selected_entity = entity;

   auto &panel = EdContext.entity_panel;
   panel.active = true;
   panel.entity = entity;
   panel.reverse_scale_x = false;
   panel.reverse_scale_y = false;
   panel.reverse_scale_z = false;
   // panel.x_arrow->rotation = vec3{0,0,270};
   // panel.y_arrow->rotation = vec3{0,0,0};
   // panel.z_arrow->rotation = vec3{90,0,0};
   panel.show_normals = false;
   panel.show_collider = false;
   panel.show_bounding_box = false;
   panel.entity_tracked_state = get_entity_state(entity);
}
