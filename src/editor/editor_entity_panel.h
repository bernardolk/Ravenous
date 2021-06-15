// -------------
// ENTITY PANEL
// -------------
void undo_entity_panel_changes();
void undo_selected_entity_move_changes();
void set_entity_panel(Entity* entity);
void check_for_asset_changes();
void update_entity_control_arrows(EntityPanelContext* panel);
void render_entity_control_arrows(EntityPanelContext* panel);
void check_selection_to_open_panel();
void render_entity_panel(EntityPanelContext* panel);

void render_entity_panel(EntityPanelContext* panel)
{
   auto& entity = panel->entity;
   ImGui::SetNextWindowPos(ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH - 300, 370), ImGuiCond_Appearing);
   ImGui::Begin("Entity Panel", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);

   ImGui::Text(entity->name.c_str());

   //rename
   ImGui::NewLine();
   if(ImGui::InputText("rename", &panel->rename_buffer[0], 100))
      entity->name = panel->rename_buffer;

   ImGui::NewLine();
   // position
   bool used_pos = false;
   {
      bool pos_x = ImGui::SliderFloat(
         "x",
         &entity->position.x,
         panel->original_position.x - 4,
         panel->original_position.x + 4
      );
      bool pos_y = ImGui::SliderFloat(
         "y",
         &entity->position.y,
         panel->original_position.y - 4,
         panel->original_position.y + 4
      );
      bool pos_z = ImGui::SliderFloat(
         "z",
         &entity->position.z,
         panel->original_position.z - 4,
         panel->original_position.z + 4
      );
      used_pos = pos_x || pos_y || pos_z;
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
   }

   ImGui::NewLine();

   // scale
   bool scaled_x = false, scaled_y = false, scaled_z = false;
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
      scaled_x = ImGui::SliderFloat(
         "scale x",
         &scale.x,
         min_scales.x,
         panel->original_scale.x + 4
      );
      if(ImGui::Checkbox("rev x", &panel->reverse_scale_x))
         panel->x_arrow->rotation.z = (int)(panel->x_arrow->rotation.z + 180) % 360;

      // scale in y
      scaled_y = ImGui::SliderFloat(
         "scale y",
         &scale.y,
         min_scales.y,
         panel->original_scale.y + 4
      );
      if(ImGui::Checkbox("rev y", &panel->reverse_scale_y))
         panel->y_arrow->rotation.z = (int)(panel->y_arrow->rotation.z + 180) % 360;

      // scale in z
      scaled_z = ImGui::SliderFloat(
         "scale z",
         &scale.z,
         min_scales.z,
         panel->original_scale.z + 4
      );
      if(ImGui::Checkbox("rev z", &panel->reverse_scale_z))
         panel->z_arrow->rotation.x = (int)(panel->z_arrow->rotation.x + 180) % 360;

      // apply scalling
      if(scaled_x || scaled_y || scaled_z)
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

   ImGui::NewLine();
   if(ImGui::CollapsingHeader("World cells"))
   {
      for(int i = 0; i < entity->world_cells_count; i++)
      {
         auto cell = entity->world_cells[i];
         ImGui::Text(cell->coordinates_str().c_str());
      }   
   }

   ImGui::NewLine();

   // Controls
   bool duplicated = false;
   bool erased = false;
   {
      if(ImGui::Button("Snap", ImVec2(82,18)))
      {
         Context.snap_mode = true;
         auto &undo_snap     = Context.entity_state_before_snap;
         undo_snap.position  = entity->position;
         undo_snap.rotation  = entity->rotation;
         undo_snap.scale     = entity->scale;
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
         select_entity_to_move_with_mouse(new_entity);
         set_entity_panel(new_entity);
      }

      if(ImGui::Button("Erase", ImVec2(82,18)))
      {
         erased = true;
         Entity_Manager.mark_for_deletion(entity);
         Context.entity_panel.active = false;
      }

      ImGui::Checkbox("Hide", &entity->wireframe);
   }

   if(used_pos || used_rot || scaled_x || scaled_y || scaled_z || duplicated || erased)
   {
      Context.snap_mode = false;
      Context.measure_mode = false;
      entity->update_collision_geometry();
      auto update_cells = World.update_entity_world_cells(entity);
      if(update_cells.status != OK)
      {
         G_BUFFERS.rm_buffer->add(update_cells.message, 3500);
      }
      World.update_cells_in_use_list();
   }

   ImGui::End();
}

void undo_entity_panel_changes()
{
   auto entity       = Context.entity_panel.entity;
   entity->position  = Context.entity_panel.original_position;
   entity->scale     = Context.entity_panel.original_scale;
   entity->rotate_y(Context.entity_panel.original_rotation - entity->rotation.y);
}

void check_selection_to_open_panel()
{
   auto pickray = cast_pickray();
   auto test = test_ray_against_scene(pickray);
   if(test.hit)
      set_entity_panel(test.entity);
}

void render_entity_control_arrows(EntityPanelContext* panel)
{
   glDepthFunc(GL_ALWAYS);
   render_editor_entity(panel->x_arrow, G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
   render_editor_entity(panel->y_arrow, G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
   render_editor_entity(panel->z_arrow, G_SCENE_INFO.active_scene, G_SCENE_INFO.camera);
   glDepthFunc(GL_LESS);
}

void set_entity_panel(Entity* entity)
{
   Context.selected_entity = entity;

   auto &undo     = Context.original_entity_state;
   undo.position  = entity->position;
   undo.rotation  = entity->rotation;
   undo.scale     = entity->scale;

   // could be made using EntityState? could, but I am lazy.
   auto &panel = Context.entity_panel;
   panel.active = true;
   panel.entity = entity;
   panel.original_position = entity->position;
   panel.original_scale    = entity->scale;
   panel.original_rotation = entity->rotation.y;
   panel.reverse_scale_x = false;
   panel.reverse_scale_y = false;
   panel.reverse_scale_z = false;
   panel.x_arrow->rotation = vec3{0,0,270};
   panel.y_arrow->rotation = vec3{0,0,0};
   panel.z_arrow->rotation = vec3{90,0,0};
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