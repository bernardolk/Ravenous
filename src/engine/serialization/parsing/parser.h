#pragma once
#include <fstream>

struct ParseUnit
{
	std::string string{};
	u32 size = 0;
	u8 has_token = 0;
	union
	{
		char string_buffer[50]{};
		int i_token;
		float f_token;
		char c_token;
		u32 ui_token;
		u64 u64_token;
		float vec3[3];
		float vec2[2];
	};

	void AdvanceChar()
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
		if (!this->reader.is_open())
			fatal_error("Couldn't open file '%s', path NOT FOUND", filepath.c_str());
	}

	Parser(const std::string& text_buffer, const int buffer_size)
	{
		this->p.string = text_buffer.c_str();
		this->p.size = buffer_size;
	}

	bool NextLine();
	void ParseWhitespace();
	void ParseAllWhitespace();
	void ParseLetter();
	void ParseSymbol();
	void ParseNameChar();
	void ParseName();
	void ParseTokenChar();
	void ParseToken();
	void ParseInt();
	void ParseUint();
	void ParseU64();
	void ParseFloat();
	void ParseVec3();
	void ParseVec2();

	void ClearParseBuffer();
	bool HasToken() const;

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
T GetParsed(Parser& parser)
{
	return *reinterpret_cast<T*>(&parser.p.i_token);
}

template<>
inline std::string GetParsed(Parser& parser)
{
	return parser.p.string_buffer;
}

template<>
inline glm::vec3 GetParsed(Parser& parser)
{
	if (parser.p.has_token == 0)
		fatal_error("FATAL: Parse has no vec3 value to be retrieved. Check line being parsed.");

	return glm::vec3{parser.p.vec3[0], parser.p.vec3[1], parser.p.vec3[2]};
}

template<>
inline glm::vec2 GetParsed(Parser& parser)
{
	if (parser.p.has_token == 0)
		fatal_error("FATAL: Parse has no vec3 value to be retrieved. Check line being parsed.");

	return glm::vec2{parser.p.vec2[0], parser.p.vec2[1]};
}
