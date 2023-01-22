#pragma once
#include "engine/serialization/parsing/parser.h"

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
} inline Console;

inline void initialize_console_buffers()
{
	auto buffers = static_cast<char**>(malloc(sizeof(char*) * Console.buffer_capacity));
	for(size_t i = 0; i < Console.buffer_capacity; i++)
	{
		buffers[i] = static_cast<char*>(calloc(Console.max_chars, sizeof(char)));
	}

	Console.buffers = buffers;
}

inline void render_console()
{
	render_text(15, GlobalDisplayConfig::viewport_height - 20, Console.scratch_buffer);
	render_text(15, GlobalDisplayConfig::viewport_height - 35, std::to_string(Console.b_ind));
}

inline void move_to_next_buffer()
{
	if(Console.b_ind < Console.current_buffer_size)
		Console.b_ind++;
	if(Console.b_ind < Console.current_buffer_size)
		copy_buffer_to_scratch_buffer();
	else
		clear_scratch_buffer();
}

inline void move_to_previous_buffer()
{
	if(Console.b_ind > 0)
		Console.b_ind--;
	if(Console.b_ind < Console.current_buffer_size)
		copy_buffer_to_scratch_buffer();
	else
		clear_scratch_buffer();
}

inline void copy_buffer_to_scratch_buffer()
{
	clear_scratch_buffer();

	int char_ind = 0;
	char scene_name[50] = {'\0'};
	while(Console.buffers[Console.b_ind][char_ind] != '\0')
	{
		Console.scratch_buffer[char_ind] = Console.buffers[Console.b_ind][char_ind];
		char_ind++;
	}

	Console.c_ind = char_ind;
}

inline void start_console_mode()
{
	auto* ES = EngineState::Get();
	ES->last_mode = ES->current_mode;
	ES->current_mode = EngineState::ProgramMode::Console;
}

inline void quit_console_mode()
{
	auto* ES = EngineState::Get();
	ES->current_mode = ES->last_mode;
	ES->last_mode = EngineState::ProgramMode::Console;
}

inline std::string commit_buffer()
{
	// copy from scratch buffer to variable
	int char_ind = 0;
	char input[50] = {'\0'};
	while(Console.scratch_buffer[char_ind] != '\0')
	{
		input[char_ind] = Console.scratch_buffer[char_ind];
		char_ind++;
	}

	// realloc if necessary
	if(Console.current_buffer_size == Console.buffer_capacity)
	{
		auto prior_capacity = Console.buffer_capacity;
		Console.buffer_capacity *= 2;
		Console.buffers = static_cast<char**>(realloc(Console.buffers, sizeof(char*) * Console.buffer_capacity));
		for(size_t i = prior_capacity; i < Console.buffer_capacity; i++)
		{
			Console.buffers[i] = static_cast<char*>(calloc(Console.max_chars, sizeof(char)));
		}
	}

	// commit to buffers (log)
	char_ind = 0;
	while(Console.scratch_buffer[char_ind] != '\0')
	{
		Console.buffers[Console.current_buffer_size][char_ind] = Console.scratch_buffer[char_ind];
		char_ind++;
	}

	// clear scratch buffer
	clear_scratch_buffer();

	// updates number of items in buffers
	Console.current_buffer_size++;
	// move buffers pointer up the stack
	Console.b_ind = Console.current_buffer_size;

	return input;
}

inline void clear_scratch_buffer()
{
	int char_ind = 0;
	while(Console.scratch_buffer[char_ind] != '\0')
	{
		Console.scratch_buffer[char_ind] = '\0';
		char_ind++;
	}
	Console.c_ind = 0;
}

inline void execute_command(const std::string& buffer_line, Player* & player, World* world, Camera* camera)
{
	Parser p{buffer_line, 50};
	p.ParseToken();
	const std::string command = get_parsed<std::string>(p);
	auto* GSI = GlobalSceneInfo::Get();

	// ---------------
	// 'SAVE' COMMAND
	// ---------------
	if(command == "save")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string argument = get_parsed<std::string>(p);
		WorldSerializer::save_to_file(argument, false);
	}

	// ---------------
	// 'COPY' COMMAND
	// ---------------
	else if(command == "copy")
	{
		// if you dont want to switch to the new file when saving scene with a new name
		p.ParseWhitespace();
		p.ParseToken();
		const std::string scene_name = get_parsed<std::string>(p);
		WorldSerializer::save_to_file(scene_name, true);
	}

	// ---------------
	// 'LOAD' COMMAND
	// ---------------
	else if(command == "load")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string scene_name = get_parsed<std::string>(p);
		// updates scene with new one
		if(WorldSerializer::load_from_file(scene_name))
		{
			player = GSI->player; // not irrelevant! do not delete
			if(EngineState::IsInEditorMode())
				player->entity_ptr->flags &= ~EntityFlags_InvisibleEntity;
			else
				player->entity_ptr->flags |= EntityFlags_InvisibleEntity;
			GConfig = ConfigSerializer::load_configs();
			GSI->active_scene->LoadConfigs(GConfig);
		}
	}

	// --------------
	// 'NEW' COMMAND
	// --------------
	else if(command == "new")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string scene_name = get_parsed<std::string>(p);
		if(scene_name != "")
		{
			auto current_scene = GSI->scene_name;
			if(WorldSerializer::check_if_scene_exists(scene_name))
			{
				Rvn::rm_buffer->Add("Scene name already exists.", 3000);
				return;
			}

			if(!WorldSerializer::load_from_file(Paths::SceneTemplate))
			{
				Rvn::rm_buffer->Add("Scene template not found.", 3000);
				return;
			}

			if(!WorldSerializer::save_to_file(scene_name, false))
			{
				// if couldnt save copy of template, falls back, so we dont edit the template by mistake
				if(WorldSerializer::load_from_file(current_scene))
				{
					assert(false); // if this happens, weird!
				}

				Rvn::rm_buffer->Add("Couldnt save new scene.", 3000);
			}

			player = GSI->player; // not irrelevant! do not delete
			if(EngineState::IsInEditorMode())
				player->entity_ptr->flags &= ~EntityFlags_InvisibleEntity;
			else
				player->entity_ptr->flags |= EntityFlags_InvisibleEntity;
		}
		else
		{
			Rvn::rm_buffer->Add("Could not create new scene. Provide a name please.", 3000);
		}
	}

	// --------------
	// 'SET' COMMAND
	// --------------
	else if(command == "set")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string argument = get_parsed<std::string>(p);
		if(argument == "scene")
		{
			GConfig.initial_scene = GSI->scene_name;
			ConfigSerializer::save(GConfig);
		}
		else if(argument == "all")
		{
			// save scene
			player->checkpoint_pos = player->entity_ptr->position;
			WorldSerializer::save_to_file();
			// set scene
			GConfig.initial_scene = GSI->scene_name;
			ConfigSerializer::save(GConfig);
		}
		else
			std::cout << "you can set 'scene' or 'all'. dude. " << command << " won't work.\n";
	}

	// -----------------
	// 'RELOAD' COMMAND
	// -----------------
	else if(command == "reload")
	{
		if(WorldSerializer::load_from_file(GSI->scene_name))
		{
			player = GSI->player; // not irrelevant! do not delete

			if(EngineState::IsInEditorMode())
				player->entity_ptr->flags &= ~EntityFlags_InvisibleEntity;
			else
				player->entity_ptr->flags |= EntityFlags_InvisibleEntity;

			GConfig = ConfigSerializer::load_configs();
			GSI->active_scene->LoadConfigs(GConfig);
			GlobalInputInfo::Get()->block_mouse_move = false;
		}
	}

	// ----------------
	// 'LIVES' COMMAND
	// ----------------
	else if(command == "lives")
	{
		player->RestoreHealth();
	}

	// ---------------
	// 'KILL' COMMAND
	// ---------------
	else if(command == "kill")
	{
		player->Die();
	}

	// ---------------
	// 'MOVE' COMMAND
	// ---------------
	else if(command == "move")
	{
		p.ParseWhitespace();
		p.ParseToken();
		const std::string argument = get_parsed<std::string>(p);
		if(argument == "cam")
		{
			p.ParseVec3();
			camera->position = get_parsed<glm::vec3>(p);
		}
		else
			std::cout << "you can move cam only at the moment dude. I don't know what '" << command << " " << argument << "' means man.\n";
	}
	else
		std::cout << "what do you mean with " << command << " man?\n";
}

inline void handle_console_input(InputFlags flags, Player* & player, World* world, Camera* camera)
{
	if(pressed_once(flags, KEY_ENTER))
	{
		// if empty, just quit
		if(Console.scratch_buffer[0] == '\0')
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

inline void check_letter_key_presses(InputFlags flags)
{
	if(pressed_once(flags, KEY_BACKSPACE))
	{
		if(Console.c_ind > 0)
			Console.scratch_buffer[--Console.c_ind] = '\0';
	}
	if(pressed_once(flags, KEY_Q))
	{
		Console.scratch_buffer[Console.c_ind++] = 'q';
	}
	if(pressed_once(flags, KEY_W))
	{
		Console.scratch_buffer[Console.c_ind++] = 'w';
	}
	if(pressed_once(flags, KEY_E))
	{
		Console.scratch_buffer[Console.c_ind++] = 'e';
	}
	if(pressed_once(flags, KEY_R))
	{
		Console.scratch_buffer[Console.c_ind++] = 'r';
	}
	if(pressed_once(flags, KEY_T))
	{
		Console.scratch_buffer[Console.c_ind++] = 't';
	}
	if(pressed_once(flags, KEY_Y))
	{
		Console.scratch_buffer[Console.c_ind++] = 'y';
	}
	if(pressed_once(flags, KEY_U))
	{
		Console.scratch_buffer[Console.c_ind++] = 'u';
	}
	if(pressed_once(flags, KEY_I))
	{
		Console.scratch_buffer[Console.c_ind++] = 'i';
	}
	if(pressed_once(flags, KEY_O))
	{
		Console.scratch_buffer[Console.c_ind++] = 'o';
	}
	if(pressed_once(flags, KEY_P))
	{
		Console.scratch_buffer[Console.c_ind++] = 'p';
	}
	if(pressed_once(flags, KEY_A))
	{
		Console.scratch_buffer[Console.c_ind++] = 'a';
	}
	if(pressed_once(flags, KEY_S))
	{
		Console.scratch_buffer[Console.c_ind++] = 's';
	}
	if(pressed_once(flags, KEY_D))
	{
		Console.scratch_buffer[Console.c_ind++] = 'd';
	}
	if(pressed_once(flags, KEY_F))
	{
		Console.scratch_buffer[Console.c_ind++] = 'f';
	}
	if(pressed_once(flags, KEY_G))
	{
		Console.scratch_buffer[Console.c_ind++] = 'g';
	}
	if(pressed_once(flags, KEY_H))
	{
		Console.scratch_buffer[Console.c_ind++] = 'h';
	}
	if(pressed_once(flags, KEY_J))
	{
		Console.scratch_buffer[Console.c_ind++] = 'j';
	}
	if(pressed_once(flags, KEY_K))
	{
		Console.scratch_buffer[Console.c_ind++] = 'k';
	}
	if(pressed_once(flags, KEY_L))
	{
		Console.scratch_buffer[Console.c_ind++] = 'l';
	}
	if(pressed_once(flags, KEY_Z))
	{
		Console.scratch_buffer[Console.c_ind++] = 'z';
	}
	if(pressed_once(flags, KEY_X))
	{
		Console.scratch_buffer[Console.c_ind++] = 'x';
	}
	if(pressed_once(flags, KEY_C))
	{
		Console.scratch_buffer[Console.c_ind++] = 'c';
	}
	if(pressed_once(flags, KEY_V))
	{
		Console.scratch_buffer[Console.c_ind++] = 'v';
	}
	if(pressed_once(flags, KEY_B))
	{
		Console.scratch_buffer[Console.c_ind++] = 'b';
	}
	if(pressed_once(flags, KEY_N))
	{
		Console.scratch_buffer[Console.c_ind++] = 'n';
	}
	if(pressed_once(flags, KEY_M))
	{
		Console.scratch_buffer[Console.c_ind++] = 'm';
	}
	if(pressed_once(flags, KEY_1))
	{
		Console.scratch_buffer[Console.c_ind++] = '1';
	}
	if(pressed_once(flags, KEY_2))
	{
		Console.scratch_buffer[Console.c_ind++] = '2';
	}
	if(pressed_once(flags, KEY_3))
	{
		Console.scratch_buffer[Console.c_ind++] = '3';
	}
	if(pressed_once(flags, KEY_4))
	{
		Console.scratch_buffer[Console.c_ind++] = '4';
	}
	if(pressed_once(flags, KEY_5))
	{
		Console.scratch_buffer[Console.c_ind++] = '5';
	}
	if(pressed_once(flags, KEY_6))
	{
		Console.scratch_buffer[Console.c_ind++] = '6';
	}
	if(pressed_once(flags, KEY_7))
	{
		Console.scratch_buffer[Console.c_ind++] = '7';
	}
	if(pressed_once(flags, KEY_8))
	{
		Console.scratch_buffer[Console.c_ind++] = '8';
	}
	if(pressed_once(flags, KEY_9))
	{
		Console.scratch_buffer[Console.c_ind++] = '9';
	}
	if(pressed_once(flags, KEY_0))
	{
		Console.scratch_buffer[Console.c_ind++] = '0';
	}
	if(pressed_once(flags, KEY_SPACE))
	{
		Console.scratch_buffer[Console.c_ind++] = ' ';
	}
}
