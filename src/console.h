void handle_console_input(InputFlags flags, Player* &player);
void check_letter_key_presses(InputFlags flags);
void clear_console_string_buffer();
void render_console();
void start_console_mode();
void quit_console_mode();
void move_to_previous_buffer();
void move_to_next_buffer();
string commit_buffer();
void initialize_console_buffers();
void copy_buffer_to_scratch_buffer();
void clear_scratch_buffer();
void execute_command(string buffer_line, Player* &player);


struct GlobalConsoleState {
   u16 buffer_capacity = 20;
   const static u16 max_chars = 50;
   char** buffers;
   u16 b_ind = 0;
   u16 current_buffer_size = 0;
   u16 buffer_size_incr = 1;
   char scratch_buffer[max_chars];
   u16 c_ind = 0;
} CONSOLE;

void initialize_console_buffers()
{
   auto buffers = (char**) malloc(sizeof(char*) * CONSOLE.buffer_capacity);
   for(size_t i = 0; i < CONSOLE.buffer_capacity; i++)
   {
      buffers[i] = (char*) calloc(CONSOLE.max_chars, sizeof(char));
   }

   CONSOLE.buffers = buffers;
}

void render_console()
{
   render_text(CONSOLE.scratch_buffer, 15, G_DISPLAY_INFO.VIEWPORT_HEIGHT - 20, 1.0);
   render_text(to_string(CONSOLE.b_ind), 15, G_DISPLAY_INFO.VIEWPORT_HEIGHT - 35, 1.0);
}

void move_to_next_buffer()
{
   if(CONSOLE.b_ind < CONSOLE.current_buffer_size)
      CONSOLE.b_ind++;
   if(CONSOLE.b_ind < CONSOLE.current_buffer_size)
      copy_buffer_to_scratch_buffer();
   else
      clear_scratch_buffer();
}

void move_to_previous_buffer()
{
   if(CONSOLE.b_ind > 0)
      CONSOLE.b_ind--;
   if(CONSOLE.b_ind < CONSOLE.current_buffer_size)
      copy_buffer_to_scratch_buffer();
    else
      clear_scratch_buffer();
}

void copy_buffer_to_scratch_buffer()
{
   clear_scratch_buffer();

   int char_ind = 0;
   char scene_name[50] = {'\0'};
   while(CONSOLE.buffers[CONSOLE.b_ind][char_ind] != '\0')
   {
      CONSOLE.scratch_buffer[char_ind] = CONSOLE.buffers[CONSOLE.b_ind][char_ind];
      char_ind++;
   }

   CONSOLE.c_ind = char_ind;
}

void start_console_mode()
{
   PROGRAM_MODE.last = PROGRAM_MODE.current;
   PROGRAM_MODE.current = CONSOLE_MODE;
}

void quit_console_mode()
{
   PROGRAM_MODE.current = PROGRAM_MODE.last; 
   PROGRAM_MODE.last = CONSOLE_MODE;
}

string commit_buffer()
{
   // copy from scratch buffer to variable
   int char_ind = 0;
   char input[50] = {'\0'};
   while(CONSOLE.scratch_buffer[char_ind] != '\0')
   {
      input[char_ind] = CONSOLE.scratch_buffer[char_ind];
      char_ind++;
   }

   // realloc if necessary
   if(CONSOLE.current_buffer_size == CONSOLE.buffer_capacity)
   {
      auto prior_capacity = CONSOLE.buffer_capacity;
      CONSOLE.buffer_capacity *= 2;
      CONSOLE.buffers = (char**) realloc(CONSOLE.buffers, sizeof(char*) * CONSOLE.buffer_capacity);
      for(size_t i = prior_capacity; i < CONSOLE.buffer_capacity; i++)
      {
         CONSOLE.buffers[i] = (char*) calloc(CONSOLE.max_chars, sizeof(char));
      }
   }

   // commit to buffers (log)
   char_ind = 0;
   while(CONSOLE.scratch_buffer[char_ind] != '\0')
   {
      CONSOLE.buffers[CONSOLE.current_buffer_size][char_ind] = CONSOLE.scratch_buffer[char_ind];
      char_ind++;
   }
   
   // clear scratch buffer
   clear_scratch_buffer();
         
   // updates number of items in buffers
   CONSOLE.current_buffer_size++;
   // move buffers pointer up the stack
   CONSOLE.b_ind = CONSOLE.current_buffer_size;

   return input;
}

void clear_scratch_buffer()
{
   int char_ind = 0;
   while(CONSOLE.scratch_buffer[char_ind] != '\0')
   {
      CONSOLE.scratch_buffer[char_ind] = '\0';
      char_ind++;
   }
   CONSOLE.c_ind = 0;
}

void execute_command(string buffer_line, Player* &player)
{
   Parser::Parse p {buffer_line.c_str(), 50};
   p = parse_token(p);
   const string command = p.string_buffer;
   if(command == "save")
   {
      p = parse_whitespace(p);
      p = parse_token(p);
      const string argument = p.string_buffer;
      // if(argument == "p")
      // {
      //    save_player_position_to_file(G_SCENE_INFO.scene_name, player);
      // }
      if(argument == "c")
      {
         save_camera_settings_to_file(
            CAMERA_FILE_PATH,
            G_SCENE_INFO.camera->Position,
            G_SCENE_INFO.camera->Front
         );
      }
      else
      {
         save_scene_to_file(argument, player, false);
      }
   }

   else if(command == "copy")
   {
      p = parse_whitespace(p);
      p = parse_token(p);
      const string scene_name = p.string_buffer;
      save_scene_to_file(scene_name, player, true);
   }
   else if(command == "load")
   {
      p = parse_whitespace(p);
      p = parse_token(p);
      const string scene_name = p.string_buffer;
      // updates scene with new one
      if(load_scene_from_file(scene_name))
      {
         player = G_SCENE_INFO.player; // not irrelevant! do not delete
         player->entity_ptr->render_me = PROGRAM_MODE.last == EDITOR_MODE ? true : false;
      }
   }
   else
   {
      cout << "what do you mean with " << command << " man?\n";
   }
}

void handle_console_input(InputFlags flags, Player* &player)
{
   if(pressed_once(flags, KEY_ENTER))
   {
      // if empty, just quit
      if(CONSOLE.scratch_buffer[0] == '\0')
      {
         quit_console_mode();
         return;
      }
      auto buffer_line = commit_buffer();
      execute_command(buffer_line, player);
      quit_console_mode();
   }

   if(pressed_once(flags, KEY_GRAVE_TICK))
   {
      quit_console_mode();
   }

   if(pressed_once(flags, KEY_UP))
   {
      move_to_previous_buffer();
   }

   if(pressed_once(flags, KEY_DOWN))
   {
      move_to_next_buffer();
   }

   // run through all letters to see if they were hit
   check_letter_key_presses(flags);

   // here we record a history for if keys were last pressed or released, so to enable smooth toggle
   G_INPUT_INFO.key_state |= flags.key_press;
   G_INPUT_INFO.key_state &= ~(flags.key_release); 
}

void check_letter_key_presses(InputFlags flags)
{
   if(pressed_once(flags, KEY_BACKSPACE))
   {
      if(CONSOLE.c_ind > 0)
         CONSOLE.scratch_buffer[--CONSOLE.c_ind] = '\0';
   }
   if(pressed_once(flags, KEY_Q))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'q';
   }
   if(pressed_once(flags, KEY_W))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'w';
   }
   if(pressed_once(flags, KEY_E))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'e';
   }
   if(pressed_once(flags, KEY_R))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'r';
   }
   if(pressed_once(flags, KEY_T))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 't';
   }
   if(pressed_once(flags, KEY_Y))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'y';
   }
   if(pressed_once(flags, KEY_U))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'u';
   }
   if(pressed_once(flags, KEY_I))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'i';
   }
   if(pressed_once(flags, KEY_O))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'o';
   }
   if(pressed_once(flags, KEY_P))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'p';
   }
   if(pressed_once(flags, KEY_A))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'a';
   }
   if(pressed_once(flags, KEY_S))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 's';
   }
   if(pressed_once(flags, KEY_D))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'd';
   }
   if(pressed_once(flags, KEY_F))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'f';
   }
   if(pressed_once(flags, KEY_G))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'g';
   }
   if(pressed_once(flags, KEY_H))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'h';
   }
   if(pressed_once(flags, KEY_J))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'j';
   }
   if(pressed_once(flags, KEY_K))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'k';
   }
   if(pressed_once(flags, KEY_L))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'l';
   }
   if(pressed_once(flags, KEY_Z))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'z';
   }
   if(pressed_once(flags, KEY_X))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'x';
   }
   if(pressed_once(flags, KEY_C))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'c';
   }
   if(pressed_once(flags, KEY_V))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'v';
   }
   if(pressed_once(flags, KEY_B))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'b';
   }
   if(pressed_once(flags, KEY_N))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'n';
   }
   if(pressed_once(flags, KEY_M))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = 'm';
   }
   if(pressed_once(flags, KEY_SPACE))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = ' ';
   }
}