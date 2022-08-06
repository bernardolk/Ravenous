
#include <engine/serialization/parsing/parser.h>


glm::vec3 Parser::get_vec3_val() const
{
	_check_has_val_error();
	return glm::vec3{p.vec3[0], p.vec3[1], p.vec3[2]};
}

glm::vec2 Parser::get_vec2_val() const
{
    _check_has_val_error();
    return glm::vec2{p.vec2[0], p.vec2[1]};
}

void Parser::_check_has_val_error() const
{
    if(p.hasToken == 0)
    {
        std::cout << "FATAL: Parse has no vec3 value to be retrieved. Check line being parsed.\n";
        assert(false);
    }
}


bool Parser::next_line()
{
	p = ParseUnit{};
	std::string line;
	if(getline(reader, line))
	{
		p.string	= line.c_str();
		p.size		= line.size();
		return true;
	}
	return false;
}


void Parser::parse_whitespace() 
{
	_clear_parse_buffer();
	
	if (p.string[0] == ' ') {
		p.iToken = 1;
		p.advance_char();
		p.hasToken = 1;
	}
}

void Parser::parse_all_whitespace() 
{
	_clear_parse_buffer();

	do{
		parse_whitespace();
	} while(p.hasToken);
}

void Parser::parse_letter() 
{
	_clear_parse_buffer();
	
	if (isalpha(p.string[0]))
   {
		p.cToken = p.string[0];
		p.advance_char();
		p.hasToken = 1;
	}
}

void Parser::parse_symbol()
{
	_clear_parse_buffer();
	
	const auto c = p.string[0];
	if (isgraph(c) && !isalnum(c) && c != ' ')
	{
		p.cToken = c;
		p.advance_char();
		p.hasToken = 1;
	}
}


void Parser::parse_name_char()
{
	// parses letters, digits, space or underline character
	_clear_parse_buffer();

	const auto c = p.string[0];
	if (isalnum(c) || c == ' ' || c == '_')
	{
		p.cToken = c;
		p.advance_char();
		p.hasToken = 1;
	}
}


void Parser::parse_name()
{
   // Names consists of alphanumeric or space chars
	_clear_parse_buffer();

	char string_buffer[50];
	size_t sb_size = 0;
	do{
		parse_name_char();
		if(p.hasToken) string_buffer[sb_size++] = p.cToken;
	} while(p.hasToken);
	
	string_buffer[sb_size] = '\0';
	if(sb_size > 0) p.hasToken = 1;
	strcpy_s(p.string_buffer, &string_buffer[0]);
}


void Parser::parse_token_char()
{
	// parses chars for tokens (without spaces)
	_clear_parse_buffer();

	const auto c = p.string[0];
	if (isalnum(c) || c == '_' || c == '.')
	{
		p.cToken = p.string[0];
		p.advance_char();
		p.hasToken = 1;
	}
}


void Parser::parse_token()
{
	// Tokens consists of alphanumeric or '_' or '.' chars
	_clear_parse_buffer();

	char string_buffer[50];
	size_t sb_size = 0;
	do{
      parse_token_char();
      if(p.hasToken) string_buffer[sb_size++] = p.cToken;
   } while(p.hasToken);

	string_buffer[sb_size] = '\0';
	if(sb_size > 0) p.hasToken = 1;
	strcpy_s(p.string_buffer, &string_buffer[0]);
}


void Parser::parse_int() 
{
	_clear_parse_buffer();

	u16 sign = 1;
	if (p.string[0] == '-') 
	{
		p.advance_char();
		sign = -1;
	}
	if (isdigit(p.string[0]))
	{
		u16 count = 0;
		char int_buf[10];

		do {
			int_buf[count++] = p.string[0];
			p.advance_char();
		} while (isdigit(p.string[0]));
		
		for(int i = 0; i < count; i++) 
			p.iToken += (int_buf[count - 1] - '0') * ten_powers[i];
		
		p.iToken *= sign;
		p.hasToken = 1;
	}
}


void Parser::parse_uint() 
{
	_clear_parse_buffer();

	if (isdigit(p.string[0]))
	{
		u16 count = 0;
		char int_buf[10];
		do {
			int_buf[count++] = p.string[0];
			p.advance_char();
		} while (isdigit(p.string[0]));
		
		for(int i = 0; i < count; i++)
			p.uiToken += (int_buf[count - 1] - '0') * ten_powers[i];
		
		p.hasToken = 1;
	}
}


void Parser::parse_u64() 
{
	_clear_parse_buffer();

	if (isdigit(p.string[0]))
	{
		u16 count = 0;
		char int_buf[15];
		do {
			int_buf[count++] = p.string[0];
			p.advance_char();
		} while (isdigit(p.string[0]) && count < 15);
		
		for(int i = 0; i < count; i++)
			p.u64Token += (int_buf[count - 1] - '0') * ten_powers[i];
		
		p.hasToken = 1;
	}
}


void Parser::parse_float() 
{
	_clear_parse_buffer();
	
	char  int_buf[10]{};
	float sign = 1.f;

	if (p.string[0] == '-') 
   {
		p.advance_char();
		sign = -1.f;
	}
	if (isdigit(p.string[0])) 
   {
	   int count = 0;
	   int fcount = 0;
	   char float_buf[10];
		do{
			int_buf[count++] = p.string[0];
			p.advance_char();
		} while (isdigit(p.string[0])); 

		if(p.string[0] == '.')
		{
			p.advance_char();
			while (isdigit(p.string[0]))
			{
				float_buf[fcount++] = p.string[0];
				p.advance_char();
			}; 
		}

		//@TODO: We are losing precision here. Investigate at some point.
		for(int i = 0; i < count; i ++) 
			p.fToken += (int_buf[count - 1] - '0') * ten_powers[i];
		
		for(int i = 0; i < fcount; i ++) 
			p.fToken += (float_buf[i] - '0') * ten_inverse_powers[i];
		
		p.fToken *= sign;
		p.hasToken = 1;
	}
}


void Parser::parse_vec3()
{
	_clear_parse_buffer();

	parse_all_whitespace();
	parse_float();
	if(!p.hasToken) return;
	const float x = p.fToken;

	parse_all_whitespace();
	parse_float();
	if(!p.hasToken) return;
	const float y = p.fToken;

	parse_all_whitespace();
	parse_float();
	if(!p.hasToken) return;
	const float z = p.fToken;

	p.vec3[0] = x;
	p.vec3[1] = y;
	p.vec3[2] = z;
}


void Parser::parse_vec2()
{
	_clear_parse_buffer();
	
	parse_all_whitespace();
	parse_float();
	if(!p.hasToken) return;
	const float u = p.fToken;

	parse_all_whitespace();
	parse_float();
	if(!p.hasToken) return;
	const float v = p.fToken;

	p.vec2[0] = u;
	p.vec2[1] = v;
}

void Parser::_clear_parse_buffer()
{
	p = ParseUnit{.string = p.string, .size = p.size};
}