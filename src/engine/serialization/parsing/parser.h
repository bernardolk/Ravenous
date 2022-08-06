#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <rvn_macros.h>
#include <engine/core/rvn_types.h>
#include <engine/logging.h>

struct ParseUnit {
	const char* string					= nullptr;
	size_t		size					= 0;
	u8			hasToken				= 0;
	union
	{
		char	string_buffer[50]{};
		int		iToken{};
		float	fToken{};
		char	cToken{};
		u32		uiToken{};
		u64		u64Token{};
		float	vec3[3]{};
		float	vec2[2]{};
	};

	void advance_char()
	{
		string = &(string[1]);
		size--;
	}
};

struct Parser
{
	int				line_count = 0;
	std::ifstream	reader;
	std::string		filepath;
	ParseUnit p{};

	explicit Parser(const std::string& filepath)
	{
		this->filepath	= filepath;
		this->reader	= std::ifstream(filepath);
		if(!this->reader.is_open())
		{
			std::cout << "Cant load scene from file '" + filepath + "', path NOT FOUND \n";
			assert(false);
		}
	}

	bool next_line();
	void parse_whitespace();
	void parse_all_whitespace();
	void parse_letter();
	void parse_symbol(); 
	void parse_name_char();
	void parse_name();
	void parse_token_char();
	void parse_token();
	void parse_int();
	void parse_uint();
	void parse_u64();
	void parse_float();
	void parse_vec3();
	void parse_vec2();
		
	glm::vec3 get_vec3_val() const;
	glm::vec2 get_vec2_val() const;
	
	void _check_has_val_error() const;
	void _clear_parse_buffer();

	constexpr static u32 ten_powers[10]{
		1, 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000, 1000000000
	};

	constexpr static float ten_inverse_powers[10]{
		0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f,
		0.000001f, 0.0000001f, 0.00000001f, 0.000000001f, 0.0000000001f
	};
};