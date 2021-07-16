// ---------------
// GAMEPLAY INPUT
// ---------------
void handle_movement_input(InputFlags flags, Player* &player, ProgramModeEnum pm)
{
   // assign keys
   u64 MOV_UP, MOV_DOWN, MOV_LEFT, MOV_RIGHT;
   switch(pm)
   {
      case EDITOR_MODE:
         MOV_UP    = KEY_UP;
         MOV_DOWN  = KEY_DOWN;
         MOV_LEFT  = KEY_LEFT;
         MOV_RIGHT = KEY_RIGHT;
         break;
      case GAME_MODE:
         MOV_UP    = KEY_W;
         MOV_DOWN  = KEY_S;
         MOV_LEFT  = KEY_A;
         MOV_RIGHT = KEY_D;
         break;
   }

   // reset player movement intention state
   player->dashing = false;
   auto& v = player->v_dir;
   v = vec3(0);

   // combines all key presses into one v direction
   switch(player->player_state)
   {
      case PLAYER_STATE_JUMPING:
      {
         // MID-AIR CONTROL IF JUMPING UP
         if(player->jumping_upwards)
         {
            if(pressed(flags, MOV_UP))
            {
               v += vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
            }
            if(pressed(flags, MOV_LEFT))
            {
               vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
               v -= vec3(onwards_vector.x, 0, onwards_vector.z);
            }
            if(pressed(flags, MOV_DOWN))
            {
               v -= vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
            }
            if(pressed(flags, MOV_RIGHT))
            {
               vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
               v += vec3(onwards_vector.x, 0, onwards_vector.z);
            }
         }
         break;
      }
      case PLAYER_STATE_STANDING:
      {
         // MOVE
         if(pressed(flags, MOV_UP))
         {
            v += vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
         }
         if(pressed(flags, MOV_LEFT))
         {
            vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
            v -= vec3(onwards_vector.x, 0, onwards_vector.z);
         }
         if(pressed(flags, MOV_DOWN))
         {
            v -= vec3(G_SCENE_INFO.camera->Front.x, 0, G_SCENE_INFO.camera->Front.z);
         }
         if(pressed(flags, MOV_RIGHT))
         {
            vec3 onwards_vector = glm::normalize(glm::cross(G_SCENE_INFO.camera->Front, G_SCENE_INFO.camera->Up));
            v += vec3(onwards_vector.x, 0, onwards_vector.z);
         }
         
         // DASH
         if(flags.key_press & KEY_LEFT_SHIFT)  
            player->dashing = true;
         
         // JUMP
         if (flags.key_press & KEY_SPACE) 
            make_player_jump(player);

         break;
      }
      case PLAYER_STATE_SLIDING:
      {
         auto collision_geom = player->standing_entity_ptr->collision_geometry.slope;
         v = player->slide_speed * collision_geom.tangent;

         if (flags.key_press & MOV_LEFT)
         {
            float dot_product = glm::dot(collision_geom.tangent, G_SCENE_INFO.camera->Front);
            float angle = -12.0f;
            if (dot_product < 0)
               angle *= -1;

            auto bitangent = glm::cross(collision_geom.tangent, G_SCENE_INFO.camera->Up);
            auto normal = glm::cross(bitangent, collision_geom.tangent);
            auto temp_vec = glm::rotate(v, angle, normal);
            v.x = temp_vec.x;
            v.z = temp_vec.z;
         }
         if (flags.key_press & MOV_RIGHT)
         {
            float dot_product = glm::dot(collision_geom.tangent, G_SCENE_INFO.camera->Front);
            float angle = 12.0f;
            if (dot_product < 0)
               angle *= -1;

            auto bitangent = glm::cross(collision_geom.tangent, G_SCENE_INFO.camera->Up);
            auto normal = glm::cross(bitangent, collision_geom.tangent);
            auto temp_vec = glm::rotate(v, angle, normal);
            v.x = temp_vec.x;
            v.z = temp_vec.z;
         }
         if (flags.key_press & KEY_SPACE)
         {
            vec3 n = player->standing_entity_ptr->collision_geometry.slope.normal;
            player->jumping_from_slope = true;
            v = glm::normalize(vec3(n.x, 1, n.z));
         }

         break;
      }
   }

   if(!(v.x == 0.f && v.y == 0.f && v.z == 0.f))
      v = glm::normalize(v);
}

// --------------
// SYSTEMS INPUT
// --------------
void handle_common_input(InputFlags flags, Player* &player)
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
      G_BUFFERS.rm_buffer->add("TIME STEP x1.0", 1000);
      G_FRAME_INFO.time_step = 1.0;
   }
   if(pressed_once(flags, KEY_4))
   {
      G_BUFFERS.rm_buffer->add("TIME STEP x0.3", 1000);
      G_FRAME_INFO.time_step = 0.3;
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
   if(pressed_once(flags, KEY_J))
   {
      check_trigger_interaction(player);
   }
   if(flags.key_press & KEY_ESC && flags.key_press & KEY_LEFT_SHIFT)
   {
       glfwSetWindowShouldClose(G_DISPLAY_INFO.window, true);
   }
}