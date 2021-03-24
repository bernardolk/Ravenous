
struct KeyInputFlags {
   int press;
   int release;
};

void on_mouse_btn(GLFWwindow* window, int button, int action, int mods);
void on_mouse_move(GLFWwindow* window, double xpos, double ypos);
int process_keyboard_input_key_press(GLFWwindow* window);
int process_keyboard_input_key_release(GLFWwindow* window);
void on_mouse_scroll(GLFWwindow* window, double xoffset, double yoffset);
KeyInputFlags input_phase();


// INPUT KEY PRESS FLAGS
int KEY_W               = 1 << 0;
int KEY_A               = 1 << 1;
int KEY_S               = 1 << 2;
int KEY_D               = 1 << 3;
int KEY_Q               = 1 << 4;
int KEY_E               = 1 << 5;
int KEY_O               = 1 << 6;
int KEY_LEFT            = 1 << 7;
int KEY_RIGHT           = 1 << 8;
int KEY_UP              = 1 << 9;
int KEY_DOWN            = 1 << 10;
int KEY_SPACE           = 1 << 11;
int KEY_K               = 1 << 12;
int KEY_F               = 1 << 13;
int KEY_G               = 1 << 14;
int KEY_ESC             = 1 << 15;
int KEY_LEFT_SHIFT      = 1 << 16;
int KEY_LEFT_CTRL       = 1 << 17;
int KEY_9               = 1 << 18;
int KEY_P               = 1 << 19;
int KEY_B               = 1 << 20;


// INPUT KEY RELEASE FLAGS


KeyInputFlags input_phase() 
{
		glfwPollEvents();
		auto key_press_flags = process_keyboard_input_key_press(G_DISPLAY_INFO.window);
      auto key_release_flags = process_keyboard_input_key_release(G_DISPLAY_INFO.window);
      return KeyInputFlags{key_press_flags, key_release_flags};
}


int process_keyboard_input_key_press(GLFWwindow* window)
{
   int flags = 0;

   if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      flags = flags | KEY_LEFT_SHIFT;

   if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
      flags = flags | KEY_LEFT_CTRL;

   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      flags = flags | KEY_ESC;

   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      flags = flags | KEY_W;

   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      flags = flags | KEY_A;

   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      flags = flags | KEY_S;

   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      flags = flags | KEY_D;

   if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      flags = flags | KEY_Q;

   if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      flags = flags | KEY_E;

   if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
         flags = flags | KEY_O;

   if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) 
      flags = flags | KEY_9;

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

   if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
      flags = flags | KEY_K;

   if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
      flags = flags | KEY_F;

   if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
      flags = flags | KEY_G;

   if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
      flags = flags | KEY_P;

   if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
      flags = flags | KEY_B;

   return flags;
}

int process_keyboard_input_key_release(GLFWwindow* window)
{
   int flags = 0;

   if(glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
      flags = flags | KEY_F;
   
   if(glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
      flags = flags | KEY_P;

   if(glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
      flags = flags | KEY_B;

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