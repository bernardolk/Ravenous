#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <rvn_macros.h>
#include <engine/core/rvn_types.h>
#include <engine/logging.h>
#include <engine/serialization/parsing/parser.h>

bool Parser::NextLine()
{
	if(getline(reader, p.string))
	{
		p.size = p.string.size();
		line_count++;
		return true;
	}

	reader.close();
	return false;
}

bool Parser::HasToken() const
{
	return p.has_token;
}


void Parser::ParseWhitespace()
{
	ClearParseBuffer();

	if(p.string[0] == ' ')
	{
		p.i_token = 1;
		p.AdvanceChar();
		p.has_token = 1;
	}
}

void Parser::ParseAllWhitespace()
{
	ClearParseBuffer();

	do
	{
		ParseWhitespace();
	} while(p.has_token);
}

void Parser::ParseLetter()
{
	ClearParseBuffer();

	if(isalpha(p.string[0]))
	{
		p.c_token = p.string[0];
		p.AdvanceChar();
		p.has_token = 1;
	}
}

void Parser::ParseSymbol()
{
	ClearParseBuffer();

	const auto c = p.string[0];
	if(isgraph(c) && !isalnum(c) && c != ' ')
	{
		p.c_token = c;
		p.AdvanceChar();
		p.has_token = 1;
	}
}

void Parser::ParseNameChar()
{
	// parses letters, digits, space or underline character
	ClearParseBuffer();

	const auto c = p.string[0];
	if(isalnum(c) || c == ' ' || c == '_')
	{
		p.c_token = c;
		p.AdvanceChar();
		p.has_token = 1;
	}
}

void Parser::ParseName()
{
	// Names consists of alphanumeric or space chars
	ClearParseBuffer();

	char string_buffer[50];
	size_t sb_size = 0;
	do
	{
		ParseNameChar();
		if(p.has_token)
			string_buffer[sb_size++] = p.c_token;
	} while(p.has_token);

	string_buffer[sb_size] = '\0';
	if(sb_size > 0)
		p.has_token = 1;
	strcpy_s(p.string_buffer, &string_buffer[0]);
}

void Parser::ParseTokenChar()
{
	// parses chars for tokens (without spaces)
	ClearParseBuffer();

	const auto c = p.string[0];
	if(isalnum(c) || c == '_' || c == '.')
	{
		p.c_token = p.string[0];
		p.AdvanceChar();
		p.has_token = 1;
	}
}

void Parser::ParseToken()
{
	// Tokens consists of alphanumeric or '_' or '.' chars
	ClearParseBuffer();

	char string_buffer[50];
	size_t sb_size = 0;
	do
	{
		ParseTokenChar();
		if(p.has_token)
			string_buffer[sb_size++] = p.c_token;
	} while(p.has_token);

	string_buffer[sb_size] = '\0';
	if(sb_size > 0)
		p.has_token = 1;
	strcpy_s(p.string_buffer, &string_buffer[0]);
}

void Parser::ParseInt()
{
	ClearParseBuffer();

	u16 sign = 1;
	if(p.string[0] == '-')
	{
		p.AdvanceChar();
		sign = -1;
	}
	if(isdigit(p.string[0]))
	{
		u16 count = 0;
		char int_buf[10];

		do
		{
			int_buf[count++] = p.string[0];
			p.AdvanceChar();
		} while(isdigit(p.string[0]));

		for(int i = 0; i < count; i++)
			p.i_token += (int_buf[count - (1 + i)] - '0') * ten_powers[i];

		p.i_token *= sign;
		p.has_token = 1;
	}
}

void Parser::ParseUint()
{
	ClearParseBuffer();

	if(isdigit(p.string[0]))
	{
		u16 count = 0;
		char int_buf[10];
		do
		{
			int_buf[count++] = p.string[0];
			p.AdvanceChar();
		} while(isdigit(p.string[0]));

		for(int i = 0; i < count; i++)
			p.ui_token += (int_buf[count - (1 + i)] - '0') * ten_powers[i];

		p.has_token = 1;
	}
}

void Parser::ParseU64()
{
	ClearParseBuffer();

	if(isdigit(p.string[0]))
	{
		u16 count = 0;
		char int_buf[15];
		do
		{
			int_buf[count++] = p.string[0];
			p.AdvanceChar();
		} while(isdigit(p.string[0]) && count < 15);

		for(int i = 0; i < count; i++)
			p.u64_token += (int_buf[count - (1 + i)] - '0') * ten_powers[i];

		p.has_token = 1;
	}
}

void Parser::ParseFloat()
{
	ClearParseBuffer();

	char int_buf[10]{};
	float sign = 1.f;

	if(p.string[0] == '-')
	{
		p.AdvanceChar();
		sign = -1.f;
	}

	int count = 0;
	int fcount = 0;
	char float_buf[10];
	while(isdigit(p.string[0]))
	{
		int_buf[count++] = p.string[0];
		p.AdvanceChar();
	}

	if(p.string[0] == '.')
	{
		p.AdvanceChar();
		while(isdigit(p.string[0]))
		{
			float_buf[fcount++] = p.string[0];
			p.AdvanceChar();
		}
	}

	//@TODO: We are losing precision here. Investigate at some point.
	for(int i = 0; i < count; i ++)
		p.f_token += (int_buf[count - (1 + i)] - '0') * ten_powers[i];

	for(int i = 0; i < fcount; i ++)
		p.f_token += (float_buf[i] - '0') * ten_inverse_powers[i];

	p.f_token *= sign;
	p.has_token = 1;

}

void Parser::ParseVec3()
{
	ClearParseBuffer();

	ParseAllWhitespace();
	ParseFloat();
	if(!p.has_token)
		return;
	const float x = p.f_token;

	ParseAllWhitespace();
	ParseFloat();
	if(!p.has_token)
		return;
	const float y = p.f_token;

	ParseAllWhitespace();
	ParseFloat();
	if(!p.has_token)
		return;
	const float z = p.f_token;

	p.vec3[0] = x;
	p.vec3[1] = y;
	p.vec3[2] = z;
}

void Parser::ParseVec2()
{
	ClearParseBuffer();

	ParseAllWhitespace();
	ParseFloat();
	if(!p.has_token)
		return;
	const float u = p.f_token;

	ParseAllWhitespace();
	ParseFloat();
	if(!p.has_token)
		return;
	const float v = p.f_token;

	p.vec2[0] = u;
	p.vec2[1] = v;
}

void Parser::ClearParseBuffer()
{
	p = ParseUnit{.string = p.string, .size = p.size};
}
