#pragma once
#include "engine/core/core.h"

struct InputFlags;

void handle_console_input(InputFlags flags, Player* & player, World* world, Camera* camera);
void execute_command(const std::string& buffer_line, Player* & player, World* world, Camera* camera);
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

struct GlobalConsoleState
{
	u16 buffer_capacity = 20;
	constexpr static u16 max_chars = 50;
	char** buffers;
	u16 b_ind = 0;
	u16 current_buffer_size = 0;
	u16 buffer_size_incr = 1;
	char scratch_buffer[max_chars];
	u16 c_ind = 0;
};

//todo: refactor global
inline GlobalConsoleState Console;

void initialize_console_buffers();
void render_console();
void move_to_next_buffer();
void move_to_previous_buffer();
void copy_buffer_to_scratch_buffer();
void start_console_mode();
void quit_console_mode();
std::string commit_buffer();
void clear_scratch_buffer();
void execute_command(const std::string& buffer_line, Player* & player, World* world, Camera* camera);
void handle_console_input(InputFlags flags, Player* & player, World* world, Camera* camera);
void check_letter_key_presses(InputFlags flags);
