void handle_input_flags(InputFlags flags, Player* &player, World* world, Camera* camera)
{
   // ------------------------
   // EDITOR EDITING COMMANDS
   // ------------------------
   // commands that return once detected,
   // not allowing for more than one at a time
   // to be issued.

   if(pressed(flags, KEY_LEFT_CTRL) && pressed_once(flags, KEY_Z))
   {
      // snap mode controls the undo stack while it is active.
      if(!EdContext.snap_mode)
         EdContext.undo_stack.undo();
      return;
   }

   if(pressed(flags, KEY_LEFT_CTRL) && pressed_once(flags, KEY_S))
   {
      // save scene
      player->checkpoint_pos = player->entity_ptr->position;
      save_scene_to_file("", player, world, false);
      // set scene
      G_CONFIG.initial_scene = G_SCENE_INFO.scene_name;
      save_configs_to_file();
      RVN::rm_buffer->add("world state saved", 1200);
      return;
   }

   if(pressed(flags, KEY_LEFT_CTRL) && pressed_once(flags, KEY_Y))
   {
      // snap mode controls the undo stack while it is active.
      if(!EdContext.snap_mode)
         EdContext.undo_stack.redo();
      return;
   }

   if(pressed_once(flags, KEY_ESC))
   {
      if(check_modes_are_active())     
         deactivate_editor_modes();
      else if(EdContext.entity_panel.active)  
         EdContext.entity_panel.active = false;
      else if(EdContext.world_panel.active)  
         EdContext.world_panel.active = false;
      else if(EdContext.lights_panel.active)  
         EdContext.lights_panel.active = false;
      return;
   }


   if(ImGui::GetIO().WantCaptureKeyboard)
      return;

   if(pressed_once(flags, KEY_DELETE))
   {
      if(EdContext.entity_panel.active && EdContext.entity_panel.focused)
      {
         EdContext.entity_panel.active = false;
         editor_erase_entity(EdContext.entity_panel.entity);
         return;
      }
      else if(EdContext.lights_panel.active && EdContext.lights_panel.focused)
      {
         if(EdContext.lights_panel.selected_light > -1)
         {
            editor_erase_light(EdContext.lights_panel.selected_light, EdContext.lights_panel.selected_light_type, world);
         }
         return;
      }
   }

   // ------------------------------------
   // TOOLS / CAMERA / CHARACTER CONTROLS
   // ------------------------------------

   // --------------------
   // SNAP MODE SHORTCUTS
   // --------------------
   if(EdContext.snap_mode == true)
   {
      if(pressed_once(flags, KEY_ENTER))
      {
         snap_commit();
      }
      if(pressed_only(flags, KEY_X))
      {
         if(EdContext.snap_axis == 0)
            EdContext.snap_cycle = (EdContext.snap_cycle + 1) % 3;
         else
         {
            apply_state(EdContext.snap_tracked_state);
            EdContext.snap_cycle = 0;
            EdContext.snap_axis = 0;
         }
         if(EdContext.snap_reference != nullptr)
            snap_entity_to_reference(EdContext.entity_panel.entity);
      }
      if(pressed_only(flags, KEY_Y))
      {
         if(EdContext.snap_axis == 1)
            EdContext.snap_cycle = (EdContext.snap_cycle + 1) % 3;
         else
         {
            apply_state(EdContext.snap_tracked_state);
            EdContext.snap_cycle = 0;
            EdContext.snap_axis = 1;
         }
         if(EdContext.snap_reference != nullptr)
            snap_entity_to_reference(EdContext.entity_panel.entity);
      }
      if(pressed_only(flags, KEY_Z))
      {
         if(EdContext.snap_axis == 2)
            EdContext.snap_cycle = (EdContext.snap_cycle + 1) % 3;
         else
         {
            apply_state(EdContext.snap_tracked_state);
            EdContext.snap_cycle = 0;
            EdContext.snap_axis = 2;
         }
         if(EdContext.snap_reference != nullptr)
            snap_entity_to_reference(EdContext.entity_panel.entity);
      }
      if(pressed_only(flags, KEY_I))
      {
         EdContext.snap_inside = !EdContext.snap_inside;
         if(EdContext.snap_reference != nullptr)
            snap_entity_to_reference(EdContext.entity_panel.entity);
      }
   }

   // --------------------
   // MOVE MODE SHORTCUTS
   // --------------------
   if(EdContext.move_mode == true)
   {
      if(pressed(flags, KEY_X) && pressed(flags, KEY_Z))
      {
         EdContext.move_axis = 0;
      }
      if(pressed_only(flags, KEY_X))
      {
         EdContext.move_axis = 1;
      }
      if(pressed_only(flags, KEY_Y))
      {
         EdContext.move_axis = 2;
      }
      if(pressed_only(flags, KEY_Z))
      {
         EdContext.move_axis = 3;
      }
      if(pressed_only(flags, KEY_M))
      {
         EdContext.move_mode = false;
         EdContext.place_mode = true;
         return;
      }
   }

   // ---------------------
   // PLACE MODE SHORTCUTS
   // ---------------------
   if(EdContext.place_mode == true)
   {
      if(pressed_only(flags, KEY_M))
      {
         EdContext.place_mode = false;
         EdContext.move_mode = true;
         return;
      }
   }

   // -------------------
   // CAMERA TYPE TOGGLE
   // -------------------
   if(pressed_once(flags, KEY_T))
   {  // toggle camera type
      if (G_SCENE_INFO.camera->type == FREE_ROAM)
         set_camera_to_third_person(G_SCENE_INFO.camera);
      else if (G_SCENE_INFO.camera->type == THIRD_PERSON)
         set_camera_to_free_roam(G_SCENE_INFO.camera);
   }

   // ---------------
   // CLICK CONTROLS
   // ---------------

   // TODO: Refactor this whole thing: This checks for a CLICK then for modes to decide what todo.
   // I think this is not the best way to handle this, we should first check for MODE then for ACTION.
   // We can set things in EdContext based on input flags, OR use flags directly.
   // Either way, there is code for handling clicks and etc both here and at editor_main which is confusing
   // and is, currently, causing some bugs in the editor.

   EdContext.mouse_click      = false;
   EdContext.mouse_dragging   = false;

   if(G_INPUT_INFO.mouse_state & MOUSE_LB_CLICK)
   {
      std::cout << "CLICK COUNT\n";
      if(EdContext.snap_mode)
      {
         check_selection_to_snap();
      }
      else if(EdContext.measure_mode)
      {
         check_selection_to_measure(world);
      }
      else if(EdContext.locate_coords_mode)
      {
         check_selection_to_locate_coords(world);
      }
      else if(EdContext.stretch_mode)
      {
         check_selection_to_stretch();
      }
      else if(flags.key_press & KEY_G)
      {
         check_selection_to_move_entity(world, camera);
      }
      else
      {
         EdContext.mouse_click = true;

         if(EdContext.entity_panel.active)
         {
            if(EdContext.select_entity_aux_mode) return;
            if(check_selection_to_grab_entity_arrows(camera)) return;
            if(check_selection_to_grab_entity_rotation_gizmo(camera)) return;
         }

         if(EdContext.move_mode || EdContext.place_mode) return;

         check_selection_to_open_panel(player, world, camera);
      }
   }

   else if(G_INPUT_INFO.mouse_state & MOUSE_LB_DRAGGING)
   {
      EdContext.mouse_dragging = true;
   }
   else if(G_INPUT_INFO.mouse_state & MOUSE_LB_HOLD)
   {
      EdContext.mouse_dragging = true;
   }
   

   // -------------------------------
   // SPAWN PLAYER ON MOUSE POSITION
   // -------------------------------
   if(pressed_once(flags, KEY_C))
   {
      auto pickray = cast_pickray(G_SCENE_INFO.camera, G_INPUT_INFO.mouse_coords.x, G_INPUT_INFO.mouse_coords.y);
      auto test = world->raycast(pickray, player->entity_ptr);
      if(test.hit)
      {
         auto surface_point =  point_from_detection(pickray, test);
         player->entity_ptr->position = surface_point;
         player->player_state = PLAYER_STATE_STANDING;
         player->standing_entity_ptr = test.entity;
         player->entity_ptr->velocity = vec3(0, 0, 0);
         player->update(world);
      }
   }

   // --------------------------------------------
   // CONTROL KEY USAGE BLOCKED FROM HERE ONWARDS
   // --------------------------------------------
   if(pressed(flags, KEY_LEFT_CTRL))         // this doesn't solve the full problem.
      return;

   // -------------------------
   // CAMERA MOVEMENT CONTROLS
   // -------------------------
   // @TODO: this sucks
   float camera_speed =
      G_SCENE_INFO.camera->type == THIRD_PERSON ?
      player->speed * RVN::frame.duration :
      RVN::frame.real_duration * edCam->Acceleration;

   if(flags.key_press & KEY_LEFT_SHIFT)
   {
      camera_speed = camera_speed * 2;
   }

   if(flags.key_press & KEY_W)
   {
      G_SCENE_INFO.camera->Position += camera_speed * G_SCENE_INFO.camera->Front;
   }
   if(flags.key_press & KEY_A)
   {
      // @TODO: this sucks too
      if(G_SCENE_INFO.camera->type == FREE_ROAM)
         G_SCENE_INFO.camera->Position -= camera_speed * glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
      else if(G_SCENE_INFO.camera->type == THIRD_PERSON)
         G_SCENE_INFO.camera->orbital_angle -= 0.025;
   }
   if(pressed(flags, KEY_S))
   {
      G_SCENE_INFO.camera->Position -= camera_speed * G_SCENE_INFO.camera->Front;
   }
   if(flags.key_press & KEY_D)
   {
      if(G_SCENE_INFO.camera->type == FREE_ROAM)
         G_SCENE_INFO.camera->Position += camera_speed * glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
      else if(G_SCENE_INFO.camera->type == THIRD_PERSON)
         G_SCENE_INFO.camera->orbital_angle += 0.025;
   }
   if(flags.key_press & KEY_Q)
   {
      G_SCENE_INFO.camera->Position -= camera_speed * G_SCENE_INFO.camera->Up;
   }
      if(flags.key_press & KEY_E)
   {
      G_SCENE_INFO.camera->Position += camera_speed * G_SCENE_INFO.camera->Up;
   }
   if(flags.key_press & KEY_O)
   {
      camera_look_at(G_SCENE_INFO.camera, vec3(0.0f, 0.0f, 0.0f), true);
   }
}