void handle_input_flags(InputFlags flags, Player* &player)
{
   if(pressed(flags, KEY_LEFT_CTRL) && pressed_once(flags, KEY_Z))
   {
      // snap mode controls the undo stack while it is active.
      if(!Context.snap_mode)
         Context.undo_stack.undo();
   }

   if(pressed(flags, KEY_LEFT_CTRL) && pressed_once(flags, KEY_Y))
   {
      // snap mode controls the undo stack while it is active.
      if(!Context.snap_mode)
         Context.undo_stack.redo();
   }

   if(pressed_once(flags, KEY_ESC))
   {
      if(Context.snap_mode) Context.snap_mode = false;
      else if(Context.measure_mode) Context.measure_mode = false;
      else if(Context.entity_panel.active) Context.entity_panel.active = false;
   }


   if(ImGui::GetIO().WantCaptureKeyboard)
      return;

   if(pressed_once(flags, KEY_DELETE))
   {
      if(Context.entity_panel.active)
      {
         Context.entity_panel.active = false;
         editor_erase_entity(Context.entity_panel.entity);
      }
   }

   // --------------------
   // SNAP MODE SHORTCUTS
   // --------------------
   if(Context.snap_mode == true)
   {
      if(pressed_once(flags, KEY_ENTER))
      {
         snap_commit();
      }
      if(pressed_only(flags, KEY_X))
      {
         if(Context.snap_axis == 0)
            Context.snap_cycle = (Context.snap_cycle + 1) % 3;
         else
         {
            apply_state(Context.snap_tracked_state);
            Context.snap_cycle = 0;
            Context.snap_axis = 0;
         }
         if(Context.snap_reference != nullptr)
            snap_entity_to_reference(Context.entity_panel.entity);
      }
      if(pressed_only(flags, KEY_Y))
      {
         if(Context.snap_axis == 1)
            Context.snap_cycle = (Context.snap_cycle + 1) % 3;
         else
         {
            apply_state(Context.snap_tracked_state);
            Context.snap_cycle = 0;
            Context.snap_axis = 1;
         }
         if(Context.snap_reference != nullptr)
            snap_entity_to_reference(Context.entity_panel.entity);
      }
      if(pressed_only(flags, KEY_Z))
      {
         if(Context.snap_axis == 2)
            Context.snap_cycle = (Context.snap_cycle + 1) % 3;
         else
         {
            apply_state(Context.snap_tracked_state);
            Context.snap_cycle = 0;
            Context.snap_axis = 2;
         }
         if(Context.snap_reference != nullptr)
            snap_entity_to_reference(Context.entity_panel.entity);
      }
      if(pressed_only(flags, KEY_I))
      {
         Context.snap_inside = !Context.snap_inside;
         if(Context.snap_reference != nullptr)
            snap_entity_to_reference(Context.entity_panel.entity);
      }
   }

   // --------------------
   // MOVE MODE SHORTCUTS
   // --------------------
   if(Context.move_mode == true)
   {
      if(pressed(flags, KEY_X) && pressed(flags, KEY_Z))
      {
         Context.move_axis = 0;
      }
      if(pressed_only(flags, KEY_X))
      {
         Context.move_axis = 1;
      }
      if(pressed_only(flags, KEY_Y))
      {
         Context.move_axis = 2;
      }
      if(pressed_only(flags, KEY_Z))
      {
         Context.move_axis = 3;
      }
   }

   if(pressed_once(flags, KEY_T))
   {  // toggle camera type
      if (G_SCENE_INFO.camera->type == FREE_ROAM)
         set_camera_to_third_person(G_SCENE_INFO.camera, player);
      else if (G_SCENE_INFO.camera->type == THIRD_PERSON)
         set_camera_to_free_roam(G_SCENE_INFO.camera);
   }

   // ---------------
   // CLICK CONTROLS
   // ---------------
   if(G_INPUT_INFO.mouse_state & MOUSE_LB_CLICK)
   {
      if(flags.key_press & KEY_LEFT_CTRL)
      {
         if(Context.snap_mode)
         {
            check_selection_to_snap(&Context.entity_panel);
         }
         else if(Context.measure_mode)
         {
            check_selection_to_measure();
         }
         else if(Context.locate_coords_mode)
         {
            check_selection_to_locate_coords();
         }
         else
         {
            check_selection_to_open_panel();
         }
      }
      else if(flags.key_press & KEY_G)
      {
         check_selection_to_move_entity();
      }
      else
      {
         // deselection
         Context.mouse_click = true;
      }
   }

   // -------------
   // OPEN CONSOLE
   // -------------
   if(pressed_once(flags, KEY_GRAVE_TICK))
   {
      start_console_mode();
   }

   // -------------------------------
   // SPAWN PLAYER ON MOUSE POSITION
   // -------------------------------
   if(pressed_once(flags, KEY_C))
   {
      // moves player to camera position
      // player->entity_ptr->position = G_SCENE_INFO.camera->Position + G_SCENE_INFO.camera->Front * 8.0f;
      // camera_look_at(G_SCENE_INFO.views[1], G_SCENE_INFO.camera->Front, false);
      auto pickray = cast_pickray();
      auto test = test_ray_against_scene(pickray, true);
      if(test.hit)
      {
         player->entity_ptr->position = point_from_detection(pickray, test);
         player->player_state = PLAYER_STATE_STANDING;
         player->standing_entity_ptr = test.entity;
         player->entity_ptr->velocity = vec3(0, 0, 0);
      }
   }

   // @TODO: this sucks
   float camera_speed =
      G_SCENE_INFO.camera->type == THIRD_PERSON ?
      player->speed * G_FRAME_INFO.duration * G_FRAME_INFO.time_step:
      G_FRAME_INFO.duration * G_SCENE_INFO.camera->Acceleration;

   if(flags.key_press & KEY_LEFT_SHIFT)
   {
      camera_speed = camera_speed * 2;
   }
   if(flags.key_press & KEY_LEFT_CTRL)
   {
      camera_speed = camera_speed / 2;
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
   if(flags.key_press & KEY_S)
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
   if(player->player_state == PLAYER_STATE_STANDING)
   {
      // resets velocity
      player->entity_ptr->velocity = vec3(0);

      if ((flags.key_press & KEY_UP && G_SCENE_INFO.camera->type == FREE_ROAM) ||
            (flags.key_press & KEY_W && G_SCENE_INFO.camera->type == THIRD_PERSON))
      {
         player->entity_ptr->velocity += vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
      }
      if ((flags.key_press & KEY_DOWN && G_SCENE_INFO.camera->type == FREE_ROAM) ||
            (flags.key_press & KEY_S && G_SCENE_INFO.camera->type == THIRD_PERSON))
      {
         player->entity_ptr->velocity -= vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
      }
      if (flags.key_press & KEY_LEFT)
      {
         vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
         player->entity_ptr->velocity -= vec3(onwards_vector.x, 0, onwards_vector.z);
      }
      if (flags.key_press & KEY_RIGHT)
      {
         vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
         player->entity_ptr->velocity += vec3(onwards_vector.x, 0, onwards_vector.z);
      }
      // because above we sum all combos of keys pressed, here we normalize the direction and give the movement intensity
      if(glm::length2(player->entity_ptr->velocity) > 0)
      {
         float player_frame_speed = player->speed;
         if(flags.key_press & KEY_LEFT_SHIFT)  // PLAYER DASH
            player_frame_speed *= 2;

         player->entity_ptr->velocity = player_frame_speed * glm::normalize(player->entity_ptr->velocity);
      }
      if (flags.key_press & KEY_SPACE)
      {
         player->player_state = PLAYER_STATE_JUMPING;
         player->height_before_fall = player->entity_ptr->position.y;
         player->entity_ptr->velocity.y = player->jump_initial_speed;
      }
   }
   else if(player->player_state == PLAYER_STATE_SLIDING)
   {
      auto collision_geom = player->standing_entity_ptr->collision_geometry.slope;
      player->entity_ptr->velocity = player->slide_speed * collision_geom.tangent;

      if (flags.key_press & KEY_LEFT)
      {
         auto temp_vec = glm::rotate(player->entity_ptr->velocity, -12.0f, collision_geom.normal);
         player->entity_ptr->velocity.x = temp_vec.x;
         player->entity_ptr->velocity.z = temp_vec.z;
      }
      if (flags.key_press & KEY_RIGHT)
      {
         auto temp_vec = glm::rotate(player->entity_ptr->velocity, 12.0f, collision_geom.normal);
         player->entity_ptr->velocity.x = temp_vec.x;
         player->entity_ptr->velocity.z = temp_vec.z;
      }
      if (flags.key_press & KEY_SPACE)
      {
         make_player_jump_from_slope(player);
      }
   }
}