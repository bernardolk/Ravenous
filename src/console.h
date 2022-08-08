#pragma once
#include "engine/serialization/parsing/parser.h"

void handle_console_input(InputFlags flags, Player* &player, World* world, Camera* camera);
void execute_command(const std::string& buffer_line, Player* &player, World* world, Camera* camera);
void check_letter_key_presses(InputFlags flags);
void clear_console_string_buffer();
void render_console();
void start_console_mode();
void quit_console_mode();
void move_to_previous_buffer();
void move_to_next_buffer();
std::string commit_buffer();
void initialize_console_buffers();
void copy_buffer_to_scratch_buffer();
void clear_scratch_buffer();

struct GlobalConsoleState {
   u16 buffer_capacity                 = 20;
   constexpr static u16 max_chars      = 50;
   char** buffers;   
   u16 b_ind                           = 0;
   u16 current_buffer_size             = 0;
   u16 buffer_size_incr                = 1;
   char scratch_buffer[max_chars];  
   u16 c_ind                           = 0;
} inline CONSOLE;

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
   render_text(15, GlobalDisplayConfig::VIEWPORT_HEIGHT - 20, CONSOLE.scratch_buffer);
   render_text(15, GlobalDisplayConfig::VIEWPORT_HEIGHT - 35, std::to_string(CONSOLE.b_ind));
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

std::string commit_buffer()
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

void execute_command(const std::string& buffer_line, Player* &player, World* world, Camera* camera)
{
   Parser p{buffer_line, 50};
   p.parse_token();
   const std::string command = get_parsed<std::string>(p);

   // ---------------
   // 'SAVE' COMMAND
   // ---------------
   if(command == "save")
   {
      p.parse_whitespace();
      p.parse_token();
      const std::string argument = get_parsed<std::string>(p);
      WorldSerializer::save_to_file(argument, false);
   }

   // ---------------
   // 'COPY' COMMAND
   // ---------------
   else if(command == "copy")
   {
      // if you dont want to switch to the new file when saving scene with a new name
      p.parse_whitespace();
      p.parse_token();
      const std::string scene_name = get_parsed<std::string>(p);
      WorldSerializer::save_to_file(scene_name, true);
   }

   // ---------------
   // 'LOAD' COMMAND
   // ---------------
   else if(command == "load")
   {
      p.parse_whitespace();
      p.parse_token();
      const std::string scene_name = get_parsed<std::string>(p);
      // updates scene with new one
      if(WorldSerializer::load_from_file(scene_name))
      {
         player = G_SCENE_INFO.player; // not irrelevant! do not delete
         if(PROGRAM_MODE.last == EDITOR_MODE)
            player->entity_ptr->flags &= ~EntityFlags_InvisibleEntity;
         else
            player->entity_ptr->flags |= EntityFlags_InvisibleEntity;
         G_CONFIG = ConfigSerializer::load_configs();
         G_SCENE_INFO.active_scene->load_configs(G_CONFIG);
      }
   }

   // --------------
   // 'NEW' COMMAND
   // --------------
   else if(command == "new")
   {
      p.parse_whitespace();
      p.parse_token();
      const std::string scene_name = get_parsed<std::string>(p);
      if(scene_name != "")
      {
         auto current_scene = G_SCENE_INFO.scene_name;
         if(WorldSerializer::check_if_scene_exists(scene_name))
         {
            RVN::rm_buffer->add("Scene name already exists.", 3000);
            return;
         }

         if(!WorldSerializer::load_from_file(SCENE_TEMPLATE_FILENAME))
         {
            RVN::rm_buffer->add("Scene template not found.", 3000);
            return;
         }

         if(!WorldSerializer::save_to_file(scene_name, false))
         {
            // if couldnt save copy of template, falls back, so we dont edit the template by mistake
            if(WorldSerializer::load_from_file(current_scene))
            {
               assert(false);    // if this happens, weird!
            }

            RVN::rm_buffer->add("Couldnt save new scene.", 3000);
         }

         player = G_SCENE_INFO.player; // not irrelevant! do not delete
         if(PROGRAM_MODE.last == EDITOR_MODE)
            player->entity_ptr->flags &= ~EntityFlags_InvisibleEntity;
         else
            player->entity_ptr->flags |= EntityFlags_InvisibleEntity;
      }
      else
      {
         RVN::rm_buffer->add("Could not create new scene. Provide a name please.", 3000);
      }
   }

   // --------------
   // 'SET' COMMAND
   // --------------
   else if(command == "set")
   {
      p.parse_whitespace();
      p.parse_token();
      const std::string argument = get_parsed<std::string>(p);
      if(argument == "scene")
      {
         G_CONFIG.initial_scene = G_SCENE_INFO.scene_name;
         ConfigSerializer::save(G_CONFIG);
      }
      else if(argument == "all")
      {
         // save scene
         player->checkpoint_pos = player->entity_ptr->position;
         WorldSerializer::save_to_file();
         // set scene
         G_CONFIG.initial_scene = G_SCENE_INFO.scene_name;
         ConfigSerializer::save(G_CONFIG);
      }
      else std::cout << "you can set 'scene' or 'all'. dude. " << command << " won't work.\n";
   }

   // -----------------
   // 'RELOAD' COMMAND
   // -----------------
   else if(command == "reload")
   {
      if(WorldSerializer::load_from_file(G_SCENE_INFO.scene_name))
      {
         player = G_SCENE_INFO.player; // not irrelevant! do not delete
         
         if(PROGRAM_MODE.last == EDITOR_MODE)
            player->entity_ptr->flags &= ~EntityFlags_InvisibleEntity;
         else
            player->entity_ptr->flags |= EntityFlags_InvisibleEntity;

         G_CONFIG = ConfigSerializer::load_configs();
         G_SCENE_INFO.active_scene->load_configs(G_CONFIG);
         G_INPUT_INFO.block_mouse_move = false;
      }
   }

   // ----------------
   // 'LIVES' COMMAND
   // ----------------
   else if(command == "lives")
   {
      player->restore_health();
   }

   // ---------------
   // 'KILL' COMMAND
   // ---------------
   else if(command == "kill")
   {
      player->die();
   }

   // ---------------
   // 'MOVE' COMMAND
   // ---------------
   else if(command == "move")
   {
      p.parse_whitespace();
      p.parse_token();
      const std::string argument = get_parsed<std::string>(p);
      if(argument == "cam")
      {
         p.parse_vec3();
         camera->Position = get_parsed<glm::vec3>(p);
      }
      else std::cout << "you can move cam only at the moment dude. I don't know what '" << command << " " << argument << "' means man.\n";
   }
   else std::cout << "what do you mean with " << command << " man?\n";
}

void handle_console_input(InputFlags flags, Player* &player, World* world, Camera* camera)
{
   if(pressed_once(flags, KEY_ENTER))
   {
      // if empty, just quit
      if(CONSOLE.scratch_buffer[0] == '\0')
      {
         quit_console_mode();
         return;
      }
      const auto buffer_line = commit_buffer();
      execute_command(buffer_line, player, world, camera);
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
   if(pressed_once(flags, KEY_1))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '1';
   }
   if(pressed_once(flags, KEY_2))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '2';
   }
   if(pressed_once(flags, KEY_3))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '3';
   }
   if(pressed_once(flags, KEY_4))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '4';
   }
   if(pressed_once(flags, KEY_5))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '5';
   }
   if(pressed_once(flags, KEY_6))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '6';
   }
   if(pressed_once(flags, KEY_7))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '7';
   }
   if(pressed_once(flags, KEY_8))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '8';
   }
   if(pressed_once(flags, KEY_9))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '9';
   }
   if(pressed_once(flags, KEY_0))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = '0';
   }
   if(pressed_once(flags, KEY_SPACE))
   {  
      CONSOLE.scratch_buffer[CONSOLE.c_ind++] = ' ';
   }
}