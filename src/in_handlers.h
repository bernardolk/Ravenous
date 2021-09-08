void IN_handle_common_input                     (InputFlags flags, Player* &player);
void IN_handle_movement_input                   (InputFlags flags, Player* &player, ProgramModeEnum pm);

u64 KEY_MOVE_UP, KEY_MOVE_DOWN, KEY_MOVE_LEFT, KEY_MOVE_RIGHT, KEY_ACTION, KEY_WALK;


void IN_assign_keys_to_actions(ProgramModeEnum pm)
{
   switch(pm)
   {
      case EDITOR_MODE:
         KEY_MOVE_UP          = KEY_UP;
         KEY_MOVE_DOWN        = KEY_DOWN;
         KEY_MOVE_LEFT        = KEY_LEFT;
         KEY_MOVE_RIGHT       = KEY_RIGHT;
         KEY_ACTION           = KEY_Z;
         KEY_WALK             = KEY_X;
         break;
      case GAME_MODE:
         KEY_MOVE_UP          = KEY_W;
         KEY_MOVE_DOWN        = KEY_S;
         KEY_MOVE_LEFT        = KEY_A;
         KEY_MOVE_RIGHT       = KEY_D;
         KEY_ACTION           = KEY_LEFT_SHIFT;
         KEY_WALK             = KEY_LEFT_CTRL;
         break;
   }
}


void IN_process_move_keys(InputFlags flags, vec3& v_dir)
{
   if(pressed(flags, KEY_MOVE_UP))
   {
      v_dir += nrmlz(to_xz(pCam->Front));
   }
   if(pressed(flags, KEY_MOVE_LEFT))
   {
      vec3 onwards_vector = cross(pCam->Front, pCam->Up);
      v_dir -= nrmlz(to_xz(onwards_vector));
   }
   if(pressed(flags, KEY_MOVE_DOWN))
   {
      v_dir -= nrmlz(to_xz(pCam->Front));
   }
   if(pressed(flags, KEY_MOVE_RIGHT))
   {
      vec3 onwards_vector = cross(pCam->Front, pCam->Up);
      v_dir += nrmlz(to_xz(onwards_vector));
   }
}


void IN_handle_movement_input(InputFlags flags, Player* &player, ProgramModeEnum pm)
{
   // assign keys
   IN_assign_keys_to_actions(pm);

   // reset player movement intention state
   player->dashing         = false;
   player->walking         = false;
   player->free_running    = false;
   player->action          = false;
   auto& v_dir             = player->v_dir;
   v_dir                   = vec3(0);

   // combines all key presses into one v direction
   switch(player->player_state)
   {

      case PLAYER_STATE_STANDING:
      {
         // MOVE
         IN_process_move_keys(flags, v_dir);

         // DASH
         if(flags.key_press & KEY_ACTION)  
            player->dashing = true;
         
         // WALK
         if(flags.key_press & KEY_WALK)
            player->walking = true;
         
         // JUMP
         if (flags.key_press & KEY_SPACE) 
            GP_make_player_jump(player);

         // FREE RUN
         if(pressed(flags, KEY_MOVE_UP) && pressed(flags, KEY_ACTION))
            player->free_running = true;

         break;
      }

      case PLAYER_STATE_JUMPING:
      {
         // MID-AIR CONTROL IF JUMPING UP
         if(player->jumping_upwards)
            IN_process_move_keys(flags, v_dir);

         if(pressed(flags, KEY_ACTION))
            player->action = true;

         break;
      }

      case PLAYER_STATE_FALLING:
      {
          if(pressed(flags, KEY_ACTION))
            player->action = true;

         break;
      }
      
      case PLAYER_STATE_SLIDING:
      {
         auto collision_geom = player->standing_entity_ptr->collision_geometry.slope;
         v_dir = player->slide_speed * collision_geom.tangent;

         if (flags.key_press & KEY_MOVE_LEFT)
         {
            float dot_product = glm::dot(collision_geom.tangent, G_SCENE_INFO.camera->Front);
            float angle = -12.0f;
            if (dot_product < 0)
               angle *= -1;

            auto bitangent = glm::cross(collision_geom.tangent, G_SCENE_INFO.camera->Up);
            auto normal = glm::cross(bitangent, collision_geom.tangent);
            auto temp_vec = glm::rotate(v_dir, angle, normal);
            v_dir.x = temp_vec.x;
            v_dir.z = temp_vec.z;
         }
         if (flags.key_press & KEY_MOVE_RIGHT)
         {
            float dot_product = glm::dot(collision_geom.tangent, G_SCENE_INFO.camera->Front);
            float angle = 12.0f;
            if (dot_product < 0)
               angle *= -1;

            auto bitangent = glm::cross(collision_geom.tangent, G_SCENE_INFO.camera->Up);
            auto normal = glm::cross(bitangent, collision_geom.tangent);
            auto temp_vec = glm::rotate(v_dir, angle, normal);
            v_dir.x = temp_vec.x;
            v_dir.z = temp_vec.z;
         }
         if (flags.key_press & KEY_SPACE)
         {
            vec3 n = player->standing_entity_ptr->collision_geometry.slope.normal;
            player->jumping_from_slope = true;
            v_dir = glm::normalize(vec3(n.x, 1, n.z));
         }

         break;
      }
      case PLAYER_STATE_GRABBING:
      {
         if(pressed(flags, KEY_ACTION))
         {
            player->action = true;

            if(pressed(flags, KEY_MOVE_UP))
               GP_make_player_get_up_from_edge(player);
         }
            
         break;
      }
   }

   if(!(v_dir.x == 0.f && v_dir.y == 0.f && v_dir.z == 0.f))
      v_dir = glm::normalize(v_dir);
}


// --------------
// SYSTEMS INPUT
// --------------
void IN_handle_common_input(InputFlags flags, Player* &player)
{
   if(pressed_once(flags, KEY_COMMA))
   {
      if(G_FRAME_INFO.time_step > 0)
      {
         G_FRAME_INFO.time_step -= 0.025; 
      }
   }
   if(pressed_once(flags, KEY_PERIOD))
   {
      if(G_FRAME_INFO.time_step < 3)
      {
         G_FRAME_INFO.time_step += 0.025;
      }
   }
   if(pressed_once(flags, KEY_1))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x0.05", 1000);
      G_FRAME_INFO.time_step = 0.05;
   }
   if(pressed_once(flags, KEY_2))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x0.1", 1000);
      G_FRAME_INFO.time_step = 0.1;
   }
   if(pressed_once(flags, KEY_3))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x0.3", 1000);
      G_FRAME_INFO.time_step = 0.3;
   }
   if(pressed_once(flags, KEY_4))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x1.0", 1000);
      G_FRAME_INFO.time_step = 1.0;
   }
   if(pressed_once(flags, KEY_5))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x2.0", 1000);
      G_FRAME_INFO.time_step = 2.0;
   }
   if(flags.key_press & KEY_K)
   {
      player->die();
   }
   if(pressed_once(flags, KEY_F))
   {
      toggle_program_modes(player);
   }
   if(pressed_once(flags, KEY_GRAVE_TICK))
   {
      start_console_mode();
   }
   if(pressed_once(flags, KEY_J))
   {
      GP_check_trigger_interaction(player);
   }
   if(flags.key_press & KEY_ESC && flags.key_press & KEY_LEFT_SHIFT)
   {
       glfwSetWindowShouldClose(G_DISPLAY_INFO.window, true);
   }
}