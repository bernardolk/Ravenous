
void on_mouse_btn(GLFWwindow* window, int button, int action, int mods);
void on_mouse_move(GLFWwindow* window, double xpos, double ypos);
void process_keyboard_input(GLFWwindow* window);
void on_mouse_scroll(GLFWwindow* window, double xoffset, double yoffset);
void input_phase();

void input_phase() {
		glfwPollEvents();
		process_keyboard_input(G_DISPLAY_INFO.window);
}


void process_keyboard_input(GLFWwindow* window)
{
	//Todo: get a real input toggling system in place
	// something that allows you to wait for release to get in the if again
	float cameraSpeed = G_FRAME_INFO.delta_time * G_SCENE_INFO.camera.Acceleration;

   if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
      cameraSpeed = cameraSpeed * 2;
   }
   if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
      cameraSpeed = cameraSpeed / 2.0;
   }
   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      G_SCENE_INFO.camera.Position += cameraSpeed * G_SCENE_INFO.camera.Front;
   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      G_SCENE_INFO.camera.Position -= cameraSpeed * G_SCENE_INFO.camera.Front;
   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      G_SCENE_INFO.camera.Position -= cameraSpeed * glm::normalize(glm::cross(G_SCENE_INFO.camera.Front, G_SCENE_INFO.camera.Up));
   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      G_SCENE_INFO.camera.Position += cameraSpeed * glm::normalize(glm::cross(G_SCENE_INFO.camera.Front, G_SCENE_INFO.camera.Up));
   if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      G_SCENE_INFO.camera.Position -= cameraSpeed * G_SCENE_INFO.camera.Up;
   if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      G_SCENE_INFO.camera.Position += cameraSpeed * G_SCENE_INFO.camera.Up;
   if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
      camera_look_at(G_SCENE_INFO.camera, glm::vec3(0.0f, 0.0f, 0.0f), true);
      G_INPUT_INFO.reset_mouse_coords = true;
   }
   if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
      save_camera_settings_to_file("w:/camera.txt", G_SCENE_INFO.camera.Position, G_SCENE_INFO.camera.Front);
   }


	// This solution will only work while i have only one key combo implemented (i guess)
	if (G_INPUT_INFO.key_combo_pressed) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
			G_INPUT_INFO.key_combo_pressed = false;
	}

}

void on_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
   if (G_INPUT_INFO.is_mouse_drag) {
      // 'teleports' stored coordinates to current mouse coordinates
      if (G_INPUT_INFO.reset_mouse_coords) {
         G_INPUT_INFO.last_registered_mouse_coord_x = xpos;
         G_INPUT_INFO.last_registered_mouse_coord_y = ypos;
         G_INPUT_INFO.reset_mouse_coords = false;
      }

      // calculates offsets
      float xoffset = xpos - G_INPUT_INFO.last_registered_mouse_coord_x;
      float yoffset = G_INPUT_INFO.last_registered_mouse_coord_y - ypos;
      G_INPUT_INFO.last_registered_mouse_coord_x = xpos;
      G_INPUT_INFO.last_registered_mouse_coord_y = ypos;

      xoffset *= G_SCENE_INFO.camera.Sensitivity;
      yoffset *= G_SCENE_INFO.camera.Sensitivity;

      camera_change_direction(G_SCENE_INFO.camera, xoffset, yoffset);

      // Unallows camera to perform a flip
      if (G_SCENE_INFO.camera.Pitch > 89.0f)
         G_SCENE_INFO.camera.Pitch = 89.0f;
      if (G_SCENE_INFO.camera.Pitch < -89.0f)
         G_SCENE_INFO.camera.Pitch = -89.0f;

      // Make sure we don't overflow floats when camera is spinning indefinetely
      if (G_SCENE_INFO.camera.Yaw > 360.0f)
         G_SCENE_INFO.camera.Yaw = G_SCENE_INFO.camera.Yaw - 360.0f;
      if (G_SCENE_INFO.camera.Yaw < -360.0f)
         G_SCENE_INFO.camera.Yaw = G_SCENE_INFO.camera.Yaw + 360.0f;
   }

   G_INPUT_INFO.currentMouseX = xpos;
	G_INPUT_INFO.currentMouseY = ypos;

	// mouse dragging controls
	if (G_INPUT_INFO.is_mouse_left_btn_press
		&& (abs(G_INPUT_INFO.mouse_btn_down_x - G_INPUT_INFO.currentMouseX) > 2
			|| abs(G_INPUT_INFO.mouse_btn_down_y - G_INPUT_INFO.currentMouseY) > 2)) {
		G_INPUT_INFO.is_mouse_drag = true;
	}
}

void on_mouse_scroll(GLFWwindow* window, double xoffset, double yoffset) 
{
		G_SCENE_INFO.camera.Position += (float)(3 * yoffset) * G_SCENE_INFO.camera.Front;
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
      cout << "left_btn_release" << endl;
      G_INPUT_INFO.is_mouse_left_btn_press = false;
      cout << G_INPUT_INFO.is_mouse_drag << endl;
      G_INPUT_INFO.is_mouse_drag = false;
   }
}