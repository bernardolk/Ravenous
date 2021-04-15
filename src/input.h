
struct KeyInputFlags {
   u64 press;
   u64 release;
};

void on_mouse_btn(GLFWwindow* window, int button, int action, int mods);
void on_mouse_move(GLFWwindow* window, double xpos, double ypos);
u64 process_keyboard_input_key_press(GLFWwindow* window);
u64 process_keyboard_input_key_release(GLFWwindow* window);
void on_mouse_scroll(GLFWwindow* window, double xoffset, double yoffset);
KeyInputFlags input_phase();
bool press_once(KeyInputFlags flags, u64 key);

u64 KEY_Q               = 1LL << 0;
u64 KEY_W               = 1LL << 1;
u64 KEY_E               = 1LL << 2;
u64 KEY_R               = 1LL << 3;
u64 KEY_T               = 1LL << 4;
u64 KEY_Y               = 1LL << 5;
u64 KEY_U               = 1LL << 6;
u64 KEY_I               = 1LL << 7;
u64 KEY_O               = 1LL << 8;
u64 KEY_P               = 1LL << 9;
u64 KEY_A               = 1LL << 10;
u64 KEY_S               = 1LL << 11;
u64 KEY_D               = 1LL << 12;
u64 KEY_F               = 1LL << 13;
u64 KEY_G               = 1LL << 14;
u64 KEY_H               = 1LL << 15;
u64 KEY_J               = 1LL << 16;
u64 KEY_K               = 1LL << 17;
u64 KEY_L               = 1LL << 18;
u64 KEY_Z               = 1LL << 19;
u64 KEY_X               = 1LL << 20;
u64 KEY_C               = 1LL << 21;
u64 KEY_V               = 1LL << 22;
u64 KEY_B               = 1LL << 23;
u64 KEY_N               = 1LL << 24;
u64 KEY_M               = 1LL << 25;
u64 KEY_0               = 1LL << 26;
u64 KEY_1               = 1LL << 27;
u64 KEY_2               = 1LL << 28;
u64 KEY_3               = 1LL << 29;
u64 KEY_4               = 1LL << 30;
u64 KEY_5               = 1LL << 31;
u64 KEY_6               = 1LL << 32;
u64 KEY_7               = 1LL << 33;
u64 KEY_8               = 1LL << 34;
u64 KEY_9               = 1LL << 35;
u64 KEY_LEFT            = 1LL << 36;
u64 KEY_RIGHT           = 1LL << 37;
u64 KEY_UP              = 1LL << 38;
u64 KEY_DOWN            = 1LL << 39;
u64 KEY_SPACE           = 1LL << 40;
u64 KEY_ESC             = 1LL << 41;
u64 KEY_LEFT_SHIFT      = 1LL << 42;
u64 KEY_LEFT_CTRL       = 1LL << 43;
u64 KEY_GRAVE_TICK      = 1LL << 44;
u64 KEY_ENTER           = 1LL << 45;
u64 KEY_BACKSPACE       = 1LL << 46;


KeyInputFlags input_phase() 
{
		glfwPollEvents();
		auto key_press_flags = process_keyboard_input_key_press(G_DISPLAY_INFO.window);
      auto key_release_flags = process_keyboard_input_key_release(G_DISPLAY_INFO.window);
      return KeyInputFlags{key_press_flags, key_release_flags};
}


u64 process_keyboard_input_key_press(GLFWwindow* window)
{
   u64 flags = 0;

   if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      flags = flags | KEY_Q;
      
   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      flags = flags | KEY_W;

   if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      flags = flags | KEY_E;

   if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
      flags = flags | KEY_R;

   if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
      flags = flags | KEY_T;

   if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
      flags = flags | KEY_Y;

   if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
      flags = flags | KEY_U;

   if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
      flags = flags | KEY_I;

   if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
      flags = flags | KEY_O;

   if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
      flags = flags | KEY_P;

   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      flags = flags | KEY_A;    

   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      flags = flags | KEY_S;

   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      flags = flags | KEY_D;

   if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
      flags = flags | KEY_F;

   if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
      flags = flags | KEY_G;

   if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
      flags = flags | KEY_H;

   if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
      flags = flags | KEY_J;

   if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
      flags = flags | KEY_K;

   if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
      flags = flags | KEY_L;

   if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
      flags = flags | KEY_Z;    

   if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
      flags = flags | KEY_X;

   if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
      flags = flags | KEY_C;

   if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
      flags = flags | KEY_V;

   if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
      flags = flags | KEY_B;

   if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
      flags = flags | KEY_N;

   if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
      flags = flags | KEY_M;

   if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) 
      flags = flags | KEY_0;

   if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) 
      flags = flags | KEY_1;

   if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) 
      flags = flags | KEY_2;

   if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) 
      flags = flags | KEY_3;

   if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) 
      flags = flags | KEY_4;

   if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) 
      flags = flags | KEY_5;

   if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) 
      flags = flags | KEY_6;

   if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) 
      flags = flags | KEY_7;

   if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) 
      flags = flags | KEY_8;

   if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) 
      flags = flags | KEY_9;

   if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      flags = flags | KEY_LEFT_SHIFT;

   if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
      flags = flags | KEY_LEFT_CTRL;

   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      flags = flags | KEY_ESC;
   
   if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
      flags = flags | KEY_UP;

   if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
      flags = flags | KEY_DOWN;
      
   if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
      flags = flags | KEY_LEFT;

   if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
      flags = flags | KEY_RIGHT;

   if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) 
      flags = flags | KEY_SPACE;

   if(glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
      flags = flags | KEY_GRAVE_TICK;

   if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
      flags = flags | KEY_ENTER;

   if(glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
      flags = flags | KEY_BACKSPACE;

   return flags;
}

u64 process_keyboard_input_key_release(GLFWwindow* window)
{
   u64 flags = 0;

   if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
      flags = flags | KEY_Q;
      
   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
      flags = flags | KEY_W;

   if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
      flags = flags | KEY_E;

   if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
      flags = flags | KEY_R;

   if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
      flags = flags | KEY_T;

   if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE)
      flags = flags | KEY_Y;

   if (glfwGetKey(window, GLFW_KEY_U) == GLFW_RELEASE)
      flags = flags | KEY_U;

   if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE)
      flags = flags | KEY_I;

   if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE)
      flags = flags | KEY_O;

   if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
      flags = flags | KEY_P;

   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)
      flags = flags | KEY_A;    

   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
      flags = flags | KEY_S;

   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
      flags = flags | KEY_D;

   if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
      flags = flags | KEY_F;

   if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
      flags = flags | KEY_G;

   if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
      flags = flags | KEY_H;

   if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE)
      flags = flags | KEY_J;

   if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE)
      flags = flags | KEY_K;

   if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
      flags = flags | KEY_L;

   if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
      flags = flags | KEY_Z;    

   if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE)
      flags = flags | KEY_X;

   if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
      flags = flags | KEY_C;

   if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE)
      flags = flags | KEY_V;

   if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
      flags = flags | KEY_B;

   if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE)
      flags = flags | KEY_N;

   if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
      flags = flags | KEY_M;

   if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE) 
      flags = flags | KEY_0;

   if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) 
      flags = flags | KEY_1;

   if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) 
      flags = flags | KEY_2;

   if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) 
      flags = flags | KEY_3;

   if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) 
      flags = flags | KEY_4;

   if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE) 
      flags = flags | KEY_5;

   if (glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE) 
      flags = flags | KEY_6;

   if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE) 
      flags = flags | KEY_7;

   if (glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) 
      flags = flags | KEY_8;

   if (glfwGetKey(window, GLFW_KEY_9) == GLFW_RELEASE) 
      flags = flags | KEY_9;

   if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE)
      flags = flags | KEY_UP;

   if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE)
      flags = flags | KEY_DOWN;
      
   if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE)
      flags = flags | KEY_LEFT;

   if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
      flags = flags | KEY_RIGHT;

   if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) 
      flags = flags | KEY_SPACE;

   if(glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_RELEASE)
      flags = flags | KEY_GRAVE_TICK;

   if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
      flags = flags | KEY_ENTER;

   if(glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE)
      flags = flags | KEY_BACKSPACE;

   return flags;
}

void on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
   if (G_INPUT_INFO.is_mouse_drag || G_SCENE_INFO.view_mode == FIRST_PERSON) 
   {
      // 'teleports' stored coordinates to current mouse coordinates
      if (G_INPUT_INFO.reset_mouse_coords) 
      {
         G_INPUT_INFO.last_registered_mouse_coord_x = xpos;
         G_INPUT_INFO.last_registered_mouse_coord_y = ypos;
         G_INPUT_INFO.reset_mouse_coords = false;
      }

      // calculates offsets
      float xoffset = xpos - G_INPUT_INFO.last_registered_mouse_coord_x;
      float yoffset = G_INPUT_INFO.last_registered_mouse_coord_y - ypos;
      G_INPUT_INFO.last_registered_mouse_coord_x = xpos;
      G_INPUT_INFO.last_registered_mouse_coord_y = ypos;

      xoffset *= G_SCENE_INFO.camera->Sensitivity;
      yoffset *= G_SCENE_INFO.camera->Sensitivity;

      camera_change_direction(G_SCENE_INFO.camera, xoffset, yoffset);

      // Unallows camera to perform a flip
      if (G_SCENE_INFO.camera->Pitch > 89.0f)
         G_SCENE_INFO.camera->Pitch = 89.0f;
      if (G_SCENE_INFO.camera->Pitch < -89.0f)
         G_SCENE_INFO.camera->Pitch = -89.0f;

      // Make sure we don't overflow floats when camera is spinning indefinetely
      if (G_SCENE_INFO.camera->Yaw > 360.0f)
         G_SCENE_INFO.camera->Yaw = G_SCENE_INFO.camera->Yaw - 360.0f;
      if (G_SCENE_INFO.camera->Yaw < -360.0f)
         G_SCENE_INFO.camera->Yaw = G_SCENE_INFO.camera->Yaw + 360.0f;

   }

   G_INPUT_INFO.currentMouseX = xpos;
	G_INPUT_INFO.currentMouseY = ypos;

	// activates mouse dragging if clicking and current mouse position has changed a certain ammount
	if (G_INPUT_INFO.is_mouse_left_btn_press)
   { 
      auto offset_from_click_x = abs(G_INPUT_INFO.mouse_btn_down_x - G_INPUT_INFO.currentMouseX);
      auto offset_from_click_y = abs(G_INPUT_INFO.mouse_btn_down_y - G_INPUT_INFO.currentMouseY); 
      if(offset_from_click_x > 2 || offset_from_click_y > 2)
      {
         G_INPUT_INFO.is_mouse_drag = true;
      }
   }
}

void on_mouse_scroll(GLFWwindow* window, double xoffset, double yoffset) 
{
		G_SCENE_INFO.camera->Position += (float)(3 * yoffset) * G_SCENE_INFO.camera->Front;
}

void on_mouse_btn(GLFWwindow* window, int button, int action, int mods) 
{
   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
   {
      G_INPUT_INFO.is_mouse_left_btn_press = true;
      G_INPUT_INFO.reset_mouse_coords = true;
      G_INPUT_INFO.mouse_btn_down_x = G_INPUT_INFO.currentMouseX;
      G_INPUT_INFO.mouse_btn_down_y = G_INPUT_INFO.currentMouseY;
   }
   else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) 
   {
      G_INPUT_INFO.is_mouse_left_btn_press = false;
      G_INPUT_INFO.is_mouse_drag = false;
   }
}

bool press_once(KeyInputFlags flags, u64 key)
{
   return flags.press & key && !(G_INPUT_INFO.key_input_state & key);
}