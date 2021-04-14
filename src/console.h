
void handle_console_input(KeyInputFlags flags, Player* &player);
void check_letter_key_presses(KeyInputFlags flags);
void clear_console_string_buffer();
void render_console();


struct GlobalConsoleState {
   char buffer[50];
   int buffer_ind = 0;
} CONSOLE_STATE;


void render_console()
{
   // NOT IMPLEMENTED YET DUE TO OPENGL SUCKING AS AN API
   render_text(CONSOLE_STATE.buffer, 15, G_DISPLAY_INFO.VIEWPORT_HEIGHT - 20, 1.0);
}

void handle_console_input(KeyInputFlags flags, Player* &player)
{
   if(press_once(flags, KEY_ENTER))
   {
      PROGRAM_MODE.current = PROGRAM_MODE.last; 
      PROGRAM_MODE.last = CONSOLE;

      // copy from buffer the scene name
      int ind = 0;
      char scene_name[50] = {'\0'};
      while(CONSOLE_STATE.buffer[ind] != '\0')
      {
         scene_name[ind] = CONSOLE_STATE.buffer[ind];
         CONSOLE_STATE.buffer[ind++] = '\0';
      }

      clear_console_string_buffer();

      // updates scene with new one
      bool loaded = load_scene_from_file(SCENES_FOLDER_PATH + scene_name + ".txt");
      if(loaded)
      {
         G_SCENE_INFO.scene_name = scene_name;
         player = G_SCENE_INFO.player; // not irrelevant! do not delete
         player->entity_ptr->render_me = G_SCENE_INFO.view_mode == FREE_ROAM ? true : false;
      }
   }
   if(press_once(flags, KEY_GRAVE_TICK))
   {
      PROGRAM_MODE.current = PROGRAM_MODE.last; 
      PROGRAM_MODE.last = CONSOLE;
   }

   // run through all letters to see if they were hit
   check_letter_key_presses(flags);

   // here we record a history for if keys were last pressed or released, so to enable smooth toggle
   G_INPUT_INFO.key_input_state |= flags.press;
   G_INPUT_INFO.key_input_state &= ~(flags.release); 
}

void check_letter_key_presses(KeyInputFlags flags)
{
   if(press_once(flags, KEY_BACKSPACE))
   {
      CONSOLE_STATE.buffer[--CONSOLE_STATE.buffer_ind] = '\0';
   }
   if(press_once(flags, KEY_Q))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'q';
   }
   if(press_once(flags, KEY_W))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'w';
   }
   if(press_once(flags, KEY_E))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'e';
   }
   if(press_once(flags, KEY_R))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'r';
   }
   if(press_once(flags, KEY_T))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 't';
   }
   if(press_once(flags, KEY_Y))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'y';
   }
   if(press_once(flags, KEY_U))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'u';
   }
   if(press_once(flags, KEY_I))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'i';
   }
   if(press_once(flags, KEY_O))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'o';
   }
   if(press_once(flags, KEY_P))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'p';
   }
   if(press_once(flags, KEY_A))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'a';
   }
   if(press_once(flags, KEY_S))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 's';
   }
   if(press_once(flags, KEY_D))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'd';
   }
   if(press_once(flags, KEY_F))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'f';
   }
   if(press_once(flags, KEY_G))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'g';
   }
   if(press_once(flags, KEY_H))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'h';
   }
   if(press_once(flags, KEY_J))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'j';
   }
   if(press_once(flags, KEY_K))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'k';
   }
   if(press_once(flags, KEY_L))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'l';
   }
   if(press_once(flags, KEY_Z))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'z';
   }
   if(press_once(flags, KEY_X))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'x';
   }
   if(press_once(flags, KEY_C))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'c';
   }
   if(press_once(flags, KEY_V))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'v';
   }
   if(press_once(flags, KEY_B))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'b';
   }
   if(press_once(flags, KEY_N))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'n';
   }
   if(press_once(flags, KEY_M))
   {  
      CONSOLE_STATE.buffer[CONSOLE_STATE.buffer_ind++] = 'm';
   }
}

void clear_console_string_buffer()
{
   int ind = 0;
   while(CONSOLE_STATE.buffer[ind] != '\0')
   {
      CONSOLE_STATE.buffer[ind++] = '\0';
   }
   CONSOLE_STATE.buffer_ind = 0;
}
