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

   string entity_identification = entity->name + " (" + to_string(entity->id) + ")";
   ImGui::Text(entity_identification.c_str());

   // entity state tracking
   bool track = false;

   // RENAME
   ImGui::NewLine();
   if(ImGui::InputText("rename", &panel->rename_buffer[0], 100))
      entity->name = panel->rename_buffer;

   // HIDE ENTITY
   ImGui::Checkbox("Hide", &entity->wireframe);

   ImGui::SameLine();
   ImGui::Checkbox("Show normals", &panel->show_normals);

   // POSITION
   ImGui::NewLine();
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

   // ROTATION
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

   // SCALE
   bool used_scaling = false;
   {
      auto scale = entity->scale;
      auto rot = glm::rotate(mat4identity, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
      scale = rot * vec4(entity->scale, 1.0f);
      auto ref_scale = scale;

      // when rotating from model to world position, scale vec gets signed
      // here we make it abs for editing in the panel
      bool flipped_x = false, flipped_z = false;
      if(scale.x < 0) { scale.x *= -1; flipped_x = true;}
      if(scale.z < 0) { scale.z *= -1; flipped_z = true;}

      // scale in x
      bool scaled_x = ImGui::DragFloat("scale x", &scale.x, 0.1);
      track = track || ImGui::IsItemDeactivatedAfterEdit();
      ImGui::Checkbox("rev x", &panel->reverse_scale_x);

      // scale in y    
      bool scaled_y = ImGui::DragFloat("scale y", &scale.y, 0.1);
      track = track || ImGui::IsItemDeactivatedAfterEdit();
      ImGui::Checkbox("rev y", &panel->reverse_scale_y);

      // scale in z
      bool scaled_z = ImGui::DragFloat("scale z", &scale.z, 0.1);
      track = track || ImGui::IsItemDeactivatedAfterEdit();
      ImGui::Checkbox("rev z", &panel->reverse_scale_z);

      used_scaling = scaled_x || scaled_y || scaled_z;

      // apply scalling
      if(used_scaling)
      {
         float min_scale =  0.001;
         scale.x = scale.x < min_scale ? min_scale : scale.x;
         scale.y = scale.y < min_scale ? min_scale : scale.y;
         scale.z = scale.z < min_scale ? min_scale : scale.z;

         // set scale orientation to its signed value before doing reverse rotation and position operations 
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
   if(ImGui::Checkbox("inside", &Context.snap_inside))
   {
      if(Context.snap_reference != nullptr)
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
         Context.entity_panel.active = false;
         editor_erase_entity(entity);
      }
      ImGui::PopStyleColor(3);
   }

   ImGui::NewLine();
   ImGui::NewLine();
   // LAYOUT END

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
   if(used_pos || used_rot || used_scaling || duplicated || deleted)
   {
      if(!(duplicated || deleted))
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
   panel.show_normals = false;
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

   if(panel->reverse_scale_x) x->rotation.z = 90;
   else x->rotation.z = 270;

   if(panel->reverse_scale_y) y->rotation.z = 180;
   else y->rotation.z = 0;

   if(panel->reverse_scale_z) z->rotation.x = 270;
   else z->rotation.x = 90;


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

void render_entity_mesh_normals(EntityPanelContext* panel)
{
   // only for aabb
   auto entity = panel->entity;

   int triangles = entity->mesh->indices.size() / 3;
   for(int i = 0; i < triangles; i++)
   {
      Triangle _t = get_triangle_for_indexed_mesh(entity, i);
      vec3 normal = glm::triangleNormal(_t.a, _t.b, _t.c);
      Face f = face_from_axis_aligned_triangle(_t);
      
      G_IMMEDIATE_DRAW.add_point(f.center, 2.0, true);

      vec3 points[] = {f.center, f.center + normal * 2.0f};
      G_IMMEDIATE_DRAW.add_line(points, 2.5, true);
   }
}