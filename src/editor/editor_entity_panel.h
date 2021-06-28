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

   string entity_identification = entity->name + " (" + to_string(entity->id) + ")";
   ImGui::Text(entity_identification.c_str());

   // entity state tracking
   bool track = false;

   //rename
   ImGui::NewLine();
   if(ImGui::InputText("rename", &panel->rename_buffer[0], 100))
      entity->name = panel->rename_buffer;

   ImGui::NewLine();
   // position
   bool used_pos = false;
   {
      bool used_x = ImGui::DragFloat("x", &entity->position.x, 0.1);
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      bool used_y = ImGui::DragFloat("y", &entity->position.y, 0.1);
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      bool used_z = ImGui::DragFloat("z", &entity->position.z, 0.1);
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      used_pos = used_x || used_y || used_z;
   }

   // rotation
   bool used_rot = false;
   {
      float rotation = entity->rotation.y;
      if(ImGui::InputFloat("rot y", &rotation, 90))
      {
         Context.entity_panel.entity->rotate_y(rotation - entity->rotation.y);
         used_rot = true;
      }
      track = track || ImGui::IsItemDeactivatedAfterEdit();
   }

   ImGui::NewLine();

   // scale
   bool used_scaling = false;
   {
      auto scale = entity->scale;
      auto rot = glm::rotate(mat4identity, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
      scale = rot * vec4(entity->scale, 1.0f);
      auto ref_scale = scale;

      bool flipped_x = false, flipped_z = false;
      if(scale.x < 0) { scale.x *= -1; flipped_x = true;}
      if(scale.z < 0) { scale.z *= -1; flipped_z = true;}

      vec3 min_scales {0.0f};

      // scale in x
      bool scaled_x = ImGui::DragFloat("scale x", &scale.x, 0.1);
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      if(ImGui::Checkbox("rev x", &panel->reverse_scale_x))
         panel->x_arrow->rotation.z = (int)(panel->x_arrow->rotation.z + 180) % 360;

      // scale in y
      bool scaled_y = ImGui::DragFloat("scale y", &scale.y,  0.1);
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      if(ImGui::Checkbox("rev y", &panel->reverse_scale_y))
         panel->y_arrow->rotation.z = (int)(panel->y_arrow->rotation.z + 180) % 360;

      // scale in z
      bool scaled_z = ImGui::DragFloat("scale z", &scale.z, 0.1);
      track = track || ImGui::IsItemDeactivatedAfterEdit();

      if(ImGui::Checkbox("rev z", &panel->reverse_scale_z))
         panel->z_arrow->rotation.x = (int)(panel->z_arrow->rotation.x + 180) % 360;

      used_scaling = scaled_x || scaled_y || scaled_z;

      // apply scalling
      if(used_scaling)
      {
         if(flipped_x) scale.x *= -1;
         if(flipped_z) scale.z *= -1;

          // if rev scaled, move entity in oposite direction to compensate scaling and fake rev scaling
         if((panel->reverse_scale_x && !flipped_x) || (flipped_x && !panel->reverse_scale_x))
            entity->position.x -= scale.x - ref_scale.x;
         if(panel->reverse_scale_y)
            entity->position.y -= scale.y - entity->scale.y;
         if((panel->reverse_scale_z && !flipped_z) || (flipped_z && !panel->reverse_scale_z))
            entity->position.z -= scale.z - ref_scale.z;

         auto inv_rot = glm::rotate(mat4identity, glm::radians(-1.0f * entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
         scale = inv_rot * vec4(scale, 1.0f);

         entity->scale = scale;
      }
   }

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

   auto shader_text = "Shader: " + entity->shader->name;
   ImGui::Text(shader_text.c_str());

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

   // if(ImGui::CollapsingHeader("Entity type"))
   // {
   //    auto is_static = entity->type == STATIC;
   //    if(ImGui::RadioButton("Static", is_static))
   //    {
   //       if(!is_static) Entity_Manager.set_type(entity, STATIC);
   //    }  

   //    auto is_checkpoint = entity->type == CHECKPOINT;
   //    if(ImGui::RadioButton("Checkpoint", is_checkpoint))
   //    {
   //       if(!is_checkpoint) Entity_Manager.set_type(entity, CHECKPOINT);
   //    }
   // }

   ImGui::NewLine();

   // Controls
   bool duplicated = false;
   bool erased = false;
   {
      if(ImGui::Button("Snap", ImVec2(82,18)))
      {
         activate_snap_mode(entity);
      }
      if(ImGui::Checkbox("inside", &Context.snap_inside))
      {
         if(Context.snap_reference != nullptr)
            snap_entity_to_reference(panel->entity);
      }

      if(ImGui::Button("Duplicate", ImVec2(82,18)))
      {
         duplicated = true;
         // entity manager logic
         auto new_entity = Entity_Manager.copy_entity(entity);
         activate_move_mode(new_entity);
         open_entity_panel(new_entity);
      }

      if(ImGui::Button("Erase", ImVec2(82,18)))
      {
         erased = true;         
         Context.entity_panel.active = false;
         editor_erase_entity(entity);
      }

      ImGui::Checkbox("Hide", &entity->wireframe);
   }

   if(track)
   {
      // we track initial state to be safe (if it wasnt tracked before)
      // since we only add to stack new states, it should be fine to add even if it is
      // already there. Currently, there is no tracking for adding/deleting entities.
      Context.undo_stack.track(panel->entity_tracked_state);
      Context.undo_stack.track(entity);
      panel->entity_tracked_state = get_entity_state(entity);
   }

   // ----------------
   // action happened
   // ----------------
   if(used_pos || used_rot || used_scaling || duplicated || erased)
   {
      if(!(duplicated || erased))
         deactivate_editor_modes();
      entity->update_collision_geometry();                              // needs to be done here to prevent a bug
      auto update_cells = World.update_entity_world_cells(entity);
      if(update_cells.status != OK)
      {
         G_BUFFERS.rm_buffer->add(update_cells.message, 3500);
      }
      World.update_cells_in_use_list();
   }

   ImGui::End();
}

void render_entity_control_arrows(EntityPanelContext* panel)
{
   glDepthFunc(GL_ALWAYS);
   render_editor_entity(panel->x_arrow, G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
   render_editor_entity(panel->y_arrow, G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
   render_editor_entity(panel->z_arrow, G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
   glDepthFunc(GL_LESS);
}

void open_entity_panel(Entity* entity)
{
   Context.selected_entity = entity;

   auto &panel = Context.entity_panel;
   panel.active = true;
   panel.entity = entity;
   panel.reverse_scale_x = false;
   panel.reverse_scale_y = false;
   panel.reverse_scale_z = false;
   panel.x_arrow->rotation = vec3{0,0,270};
   panel.y_arrow->rotation = vec3{0,0,0};
   panel.z_arrow->rotation = vec3{90,0,0};
   panel.entity_tracked_state = get_entity_state(entity);
}

void update_entity_control_arrows(EntityPanelContext* panel)
{
   auto entity = panel->entity;

   // if(entity->collision_geometry_type != COLLISION_ALIGNED_BOX)
   //    return;

   auto &x = panel->x_arrow;
   auto &y = panel->y_arrow;
   auto &z = panel->z_arrow;

   auto collision = entity->collision_geometry;

   x->position = entity->position;
   y->position = entity->position;
   z->position = entity->position;

   // change scale from local to world coordinates
   auto rot = glm::rotate(mat4identity, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
   vec3 scale_w_coords = rot * vec4(entity->scale, 1.0f);

   if(!panel->reverse_scale_x) x->position.x = max(x->position.x, x->position.x + scale_w_coords.x);
   else                        x->position.x = min(x->position.x, x->position.x + scale_w_coords.x);
   x->position.y += scale_w_coords.y / 2.0f;
   x->position.z += scale_w_coords.z / 2.0f;

   if(!panel->reverse_scale_y) y->position.y = max(y->position.y, y->position.y + scale_w_coords.y);
   else                        y->position.y = min(y->position.y, y->position.y + scale_w_coords.y);
   y->position.x += scale_w_coords.x / 2.0f;
   y->position.z += scale_w_coords.z / 2.0f;

   if(!panel->reverse_scale_z) z->position.z = max(z->position.z, z->position.z + scale_w_coords.z);
   else                        z->position.z = min(z->position.z, z->position.z + scale_w_coords.z);
   z->position.y += scale_w_coords.y / 2.0f;
   z->position.x += scale_w_coords.x / 2.0f;

   x->update_model_matrix();
   y->update_model_matrix();
   z->update_model_matrix();
}