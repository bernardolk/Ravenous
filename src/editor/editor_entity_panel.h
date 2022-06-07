// -------------
// ENTITY PANEL
// -------------
void undo_selected_entity_move_changes();
void open_entity_panel(Entity* entity);
void check_for_asset_changes();
void update_entity_control_arrows(EntityPanelContext* panel);
void render_entity_control_arrows(EntityPanelContext* panel);
void render_entity_panel(EntityPanelContext* panel);
void entity_panel_update_entity_and_editor_context(EntityPanelContext* panel, u32 action);
void entity_panel_track_entity_changes(EntityPanelContext* panel);


enum EntityPanelTrackableAction {
   EntityPanelTA_Position     = 1 << 0,
   EntityPanelTA_Rotation     = 1 << 1,
   EntityPanelTA_Scale        = 1 << 2,
   EntityPanelTA_Duplicate    = 1 << 3,
   EntityPanelTA_Delete       = 1 << 4,
};


void render_entity_panel(EntityPanelContext* panel)
{
   auto& entity = panel->entity;

   u32 action_flags = 0;
   bool track = false;

   ImGui::SetNextWindowPos(ImVec2(G_DISPLAY_INFO.VIEWPORT_WIDTH - 550, 200), ImGuiCond_Appearing);
   ImGui::Begin("Entity Panel", &panel->active, ImGuiWindowFlags_AlwaysAutoResize);
   panel->focused = ImGui::IsWindowFocused();

   ImGui::BeginTabBar("##Entity");

   // ----------------
   // > CONTROLS TAB
   // ----------------
   if(ImGui::BeginTabItem("Controls", NULL, ImGuiTabItemFlags_None))
   {
      ImGui::Text(("Name: " + entity->name).c_str());
      ImGui::Text(("Id: " + std::to_string(entity->id)).c_str());
      ImGui::Text(("Shader: " + entity->shader->name).c_str());

      // RENAME
      ImGui::NewLine();
      if(!panel->rename_option_active)
      {
         if(ImGui::Button("Rename Entity", ImVec2(120, 18)))
            panel->rename_option_active = true;
      }
      else
      {
         ImGui::InputText("New name", &panel->rename_buffer[0], 100);
         if(ImGui::Button("Apply", ImVec2(64, 18)))
         {
            if(panel->validate_rename_buffer_contents())
            {
               entity->name = panel->rename_buffer;
               panel->empty_rename_buffer();
               panel->rename_option_active = false;
            }
         }
         ImGui::SameLine();
         if(ImGui::Button("Cancel", ImVec2(64, 18)))
         {
            panel->rename_option_active = false;
         }
      }

      // HIDE ENTITY
      ImGui::SameLine();
      bool _hide_control = entity->flags & EntityFlags_HiddenEntity;
      if(ImGui::Checkbox("Hide Entity", &_hide_control))
      {
         entity->flags ^= EntityFlags_HiddenEntity;
      }

      // MODEL PROPERTIES
      ImGui::NewLine();
      ImGui::Text("Model properties:");
      ImGui::NewLine();

      
      // POSITION
      {
         float positions[]{ entity->position.x, entity->position.y, entity->position.z };
         if(ImGui::DragFloat3("Position", positions, 0.1))
         {
            action_flags |= EntityPanelTA_Position;
            entity->position = vec3{positions[0], positions[1], positions[2]};
         }
         track = track || ImGui::IsItemDeactivatedAfterEdit();
      }

      // ROTATION
      {
         float rotations[]{ entity->rotation.x, entity->rotation.y, entity->rotation.z };
         if(ImGui::DragFloat3("Rotation", rotations, 1, -360, 360))
         {
            action_flags |= EntityPanelTA_Rotation;
            entity->rotation = vec3{rotations[0], rotations[1], rotations[2]};
         }
         track = track || ImGui::IsItemDeactivatedAfterEdit();
      }

      // SCALE
      {
         float scaling[]{ entity->scale.x, entity->scale.y, entity->scale.z };
         if(ImGui::DragFloat3("Scale", scaling, 0.05, 0, MAX_FLOAT, NULL))
            action_flags |= EntityPanelTA_Scale;

         track = track || ImGui::IsItemDeactivatedAfterEdit();

         ImGui::SameLine();
         ImGui::Checkbox("Reverse", &panel->reverse_scale);

         if(action_flags & EntityPanelTA_Scale)
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

      ImGui::NewLine();

      if(ImGui::Button("Place", ImVec2(82,18)))
      {
         activate_place_mode(entity);
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

      if(ImGui::Button("Stretch", ImVec2(82,18)))
      {
         activate_stretch_mode(entity);
      }

      ImGui::NewLine();
      ImGui::NewLine();

      if(ImGui::CollapsingHeader("World cells"))
      {
         for(int i = 0; i < entity->world_cells_count; i++)
         {
            auto cell = entity->world_cells[i];
            ImGui::Text(cell->coords_str().c_str());
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
         if(ImGui::Button("Duplicate", ImVec2(82,18)))
         {
            action_flags |= EntityPanelTA_Duplicate;
            auto new_entity = Entity_Manager.copy_entity(entity);
            open_entity_panel(new_entity);
         }

         ImGui::SameLine();
         ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.03f, 0.6f, 0.6f));
         ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.03f, 0.7f, 0.7f));
         ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.03f, 0.8f, 0.8f));
         if(ImGui::Button("Delete", ImVec2(82,18)))
         {
            action_flags |= EntityPanelTA_Delete;
            EdContext.entity_panel.active = false;
            editor_erase_entity(entity);
         }
         ImGui::PopStyleColor(3);
      }

      ImGui::EndTabItem();
   }


   // -------------------
   // > ENTITY TYPE TAB
   // -------------------
   if(ImGui::BeginTabItem("Attributes", NULL, ImGuiTabItemFlags_None))
   {
      ImGui::Text("Entity Type");

      // EntityType_Static
      bool is_static = entity->type == EntityType_Static;
      if(ImGui::RadioButton("Static", is_static))
      {
         Entity_Manager.set_type(entity, EntityType_Static);
      }

      // EntityType_Checkpoint
      bool is_checkpoint = entity->type == EntityType_Checkpoint;
      if(ImGui::RadioButton("Checkpoint", is_checkpoint))
      {
         Entity_Manager.set_type(entity, EntityType_Checkpoint);
      }

      // EntityType_TimerTrigger
      bool is_timer_trigger = entity->type == EntityType_TimerTrigger;
      if(ImGui::RadioButton("Timer Trigger", is_timer_trigger))
      {
         Entity_Manager.set_type(entity, EntityType_TimerTrigger);
      }

      // EntityType_TimerTarget
      bool is_timer_target = entity->type == EntityType_TimerTarget;
      if(ImGui::RadioButton("Timer Target", is_timer_target))
      {
         Entity_Manager.set_type(entity, EntityType_TimerTarget);
      }

      // EntityType_TimerMarking
      bool is_timer_marking = entity->type == EntityType_TimerMarking;
      if(ImGui::RadioButton("Timer Marking", is_timer_marking))
      {
         Entity_Manager.set_type(entity, EntityType_TimerMarking);
      }

      ImGui::NewLine();
      ImGui::NewLine();
      ImGui::Text("Collider properties");
      ImGui::NewLine();

      ImGui::Checkbox("Slidable", &entity->slidable);

      ImGui::NewLine();
      ImGui::NewLine();

      if(!is_static)
      {
         ImGui::Text("Entity Type Properties");
         ImGui::NewLine();
      }

      if(is_checkpoint)
      {
         ImGui::Text("Event trigger");
         
         bool a = ImGui::SliderFloat("radius", &entity->trigger_scale.x, 0, 10);
         bool b = ImGui::SliderFloat("height", &entity->trigger_scale.y, 0, 10);

         if(a || b)
            entity->update();
      }

      else if(is_timer_trigger)
      {
         ImGui::Text("Event trigger");
         
         bool a = ImGui::SliderFloat("radius", &entity->trigger_scale.x, 0, 10);
         bool b = ImGui::SliderFloat("height", &entity->trigger_scale.y, 0, 10);

         if(a || b)
            entity->update();

         ImGui::NewLine();

         ImGui::SliderInt("Duration", &entity->timer_trigger_data.timer_duration, 0, 100);

         if(entity->timer_trigger_data.timer_target != nullptr)
         {
            //@todo should be any kind of time_attack_door, but ok
            if(entity->timer_trigger_data.timer_target->timer_target_data.timer_target_type == EntityTimerTargetType_VerticalSlidingDoor)
            {
               auto data = &entity->timer_trigger_data;
               int empty_slot = -1;
               bool there_is_at_least_one_marking = false;
               For(data->size)
               {
                  if(data->markings[i] != nullptr)
                  {
                     if(!there_is_at_least_one_marking)
                     {
                        // renders header for this section
                        ImGui::Text("Marking entity     -     Time checkpoint (s)     -     Delete");
                        there_is_at_least_one_marking = true;
                     }
                     ImGui::Button(data->markings[i]->name.c_str(), ImVec2(48, 18));
                     ImGui::SameLine();
                     
                     std::string dint_id = "##duration-" + std::to_string(i);
                     ImGui::DragInt(dint_id.c_str(), (int*) &data->time_checkpoints[i], 1, 0, 10000);
                     if(ImGui::Button("Delete", ImVec2(32, 18)))
                     {
                       data->delete_marking(i);
                     }
                  }
                  else if(empty_slot == -1)
                  {
                     empty_slot = i;
                  }
               }

               if(empty_slot >= 0)
               {
                  ImGui::NewLine();
            
                  if(ImGui::Button("Add marking", ImVec2(60, 18)))
                  {
                     panel->show_related_entity = false;

                     auto callback_args         = EdToolCallbackArgs();
                     callback_args.entity       = entity;
                     callback_args.entity_type  = EntityType_TimerMarking;

                     activate_select_entity_aux_tool(
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
         if(entity->timer_trigger_data.timer_target == nullptr)
            target_entity_name = "No target selected.";
         else
            target_entity_name = entity->timer_trigger_data.timer_target->name;
         ImGui::Text(target_entity_name.c_str());


         ImGui::SameLine();
         if(ImGui::Button("Show", ImVec2(68, 18)))
         {
            if(entity->timer_trigger_data.timer_target != nullptr)
            {  
               panel->show_related_entity = true;
               panel->related_entity = entity->timer_trigger_data.timer_target;
            }
         }
         ImGui::SameLine();
         if(ImGui::Button("Change", ImVec2(92, 18)))
         {
            panel->show_related_entity = false;
            activate_select_entity_aux_tool(&entity->timer_trigger_data.timer_target);
         }
      }

      ImGui::EndTabItem();
   }

   // -------------------
   // > TIMER TARGET TAB
   // -------------------
   if(entity->type == EntityType_TimerTarget)
   {
      if(ImGui::BeginTabItem("Timer Target Settings", NULL, ImGuiTabItemFlags_None))
      {
         // EntityTimerTargetType_VerticalSlidingDoor
         bool is_vsd = entity->timer_target_data.timer_target_type == EntityTimerTargetType_VerticalSlidingDoor;
         if(ImGui::RadioButton("Vertical Sliding Door", is_vsd))
         {
            entity->timer_target_data.timer_target_type = EntityTimerTargetType_VerticalSlidingDoor;
         }

         ImGui::EndTabItem();
      }
   }

   // ----------------
   // > TEXTURES TAB
   // ----------------
   if(ImGui::BeginTabItem("Textures", NULL, ImGuiTabItemFlags_None))
   {
      for(auto const& texture : Texture_Catalogue)
      {
         bool in_use = entity->textures[0].name == texture.second.name;
         if(ImGui::RadioButton(texture.second.name.c_str(), in_use))
         {
            entity->textures[0] = texture.second;
         }
      }

      bool _tiled_texture = entity->flags & EntityFlags_RenderTiledTexture;
      if(ImGui::Checkbox("Tiled texture", &_tiled_texture))
      {
         entity->flags ^= EntityFlags_RenderTiledTexture;
         if(_tiled_texture)
            entity->shader = Shader_Catalogue.find("tiledTextureModel")->second;
         else
            entity->shader = Shader_Catalogue.find("model")->second;
      }

      if(entity->flags & EntityFlags_RenderTiledTexture)
      {
         ImGui::Text("Number of tiles for each face:");
         ImGui::SliderInt("Top face",     &entity->uv_tile_wrap[0],  0, 15);
         ImGui::SliderInt("Bottom face",  &entity->uv_tile_wrap[1],  0, 15);
         ImGui::SliderInt("Front face",   &entity->uv_tile_wrap[2],  0, 15);
         ImGui::SliderInt("Left face",    &entity->uv_tile_wrap[3],  0, 15);
         ImGui::SliderInt("Right face",   &entity->uv_tile_wrap[4],  0, 15);
         ImGui::SliderInt("Back face",    &entity->uv_tile_wrap[5],  0, 15);
      }

      ImGui::EndTabItem();
   }

   // ---------------
   // > SHADERS TAB
   // ---------------
   if(ImGui::BeginTabItem("Shaders", NULL, ImGuiTabItemFlags_None))
   {
      for(auto const& shader : Shader_Catalogue)
      {
         bool in_use = entity->shader->name == shader.second->name;
         if(ImGui::RadioButton(shader.second->name.c_str(), in_use))
         {
            entity->shader = shader.second;
         }
      }

      ImGui::EndTabItem();
   }

   ImGui::EndTabBar();

   ImGui::End();

   if(action_flags > 0)
      entity_panel_update_entity_and_editor_context(panel, action_flags);
   if(track)
      entity_panel_track_entity_changes(panel);
}


void entity_panel_track_entity_changes(EntityPanelContext* panel)
{
   // the following block makes sure that we track the entity original state if necessary.
   // if we already tracked it or we used an external tool from the panel, like grab/move tool, 
   // we don't track, since these tools have their own tracking calls.
   if(!panel->tracked_once)
   {
      EntityState last_recorded_state = EdContext.undo_stack.check();
      if(last_recorded_state.entity == nullptr || last_recorded_state.entity->id != panel->entity->id)
         EdContext.undo_stack.track(panel->entity_starting_state);
      panel->tracked_once = true;
   }

   EdContext.undo_stack.track(panel->entity);
}


void entity_panel_update_entity_and_editor_context(EntityPanelContext* panel, u32 action)
{
   if(!(action & EntityPanelTA_Duplicate || action & EntityPanelTA_Delete))
      deactivate_editor_modes();

   panel->entity->update();
   auto update_cells = World.update_entity_world_cells(panel->entity);
   if(update_cells.status != CellUpdate_OK)
      G_BUFFERS.rm_buffer->add(update_cells.message, 3500);

   World.update_cells_in_use_list();
}


void open_entity_panel(Entity* entity)
{
   EdContext.selected_entity = entity;

   auto &panel                      = EdContext.entity_panel;
   panel.active                     = true;
   panel.entity                     = entity;
   panel.reverse_scale_x            = false;
   panel.reverse_scale_y            = false;
   panel.reverse_scale_z            = false;
   panel.show_normals               = false;
   panel.show_collider              = false;
   panel.show_bounding_box          = false;
   panel.rename_option_active       = false;
   panel.tracked_once               = false;
   panel.show_related_entity        = false;
   panel.related_entity             = nullptr;
   panel.entity_starting_state      = get_entity_state(entity);
   panel.empty_rename_buffer();
}
