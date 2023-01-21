#pragma once
#include <fstream>

struct ParseUnit
{
	std::string string{};
	size_t size = 0;
	u8 hasToken = 0;
	union
	{
		char string_buffer[50]{};
		int iToken;
		float fToken;
		char cToken;
		u32 uiToken;
		u64 u64Token;
		float vec3[3];
		float vec2[2];
	};

	void advance_char()
	{
		string = &(string[1]);
		size--;
	}
};

struct Parser
{
	int line_count = 0;
	std::ifstream reader;
	std::string filepath;
	ParseUnit p{};

	explicit Parser(const std::string& filepath)
	{
		this->filepath = filepath;
		this->reader = std::ifstream(filepath);
		if(!this->reader.is_open())
		{
			std::cout << "Couldn't open file '" + filepath + "', path NOT FOUND \n";
			assert(false);
		}
	}

	Parser(const std::string& text_buffer, const int buffer_size)
	{
		this->p.string = text_buffer.c_str();
		this->p.size = buffer_size;
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

	void _clear_parse_buffer();
	bool has_token() const;

	constexpr static u32 ten_powers[10]{
	1, 10, 100, 1000, 10000,
	100000, 1000000, 10000000, 100000000, 1000000000
	};

	constexpr static float ten_inverse_powers[10]{
	0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f,
	0.000001f, 0.0000001f, 0.00000001f, 0.000000001f, 0.0000000001f
	};
};

template<typename T>
T get_parsed(Parser& parser)
{
	return *reinterpret_cast<T*>(&parser.p.iToken);
}

template<>
inline std::string get_parsed(Parser& parser)
{
	return parser.p.string_buffer;
}

template<>
inline glm::vec3 get_parsed(Parser& parser)
{
	if(parser.p.hasToken == 0)
	{
		std::cout << "FATAL: Parse has no vec3 value to be retrieved. Check line being parsed.\n";
		assert(false);
	}

	return glm::vec3{parser.p.vec3[0], parser.p.vec3[1], parser.p.vec3[2]};
}

template<>
inline glm::vec2 get_parsed(Parser& parser)
{
	if(parser.p.hasToken == 0)
	{
		std::cout << "FATAL: Parse has no vec3 value to be retrieved. Check line being parsed.\n";
		assert(false);
	}

	return glm::vec2{parser.p.vec2[0], parser.p.vec2[1]};
}
