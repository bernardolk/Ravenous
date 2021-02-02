#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <Mesh.h>
#include <glm/vec3.hpp>

 struct Parse {
	const char* string;
	size_t size;
	u8 hasToken;
   char string_buffer[50];
	union{
		int iToken;
		float fToken;
		char cToken;
		unsigned int uiToken;
      float vec3[3];
	};
};

// Parse parser_start(ifstream* reader, std::string* line)
// {
//    getline(*reader, *line);
//    const char* cline = line->c_str();
//    size_t size = line->size();

//    Parse p { 
//       cline, 
//       size 
//    };

//    return p;
// }


bool parser_nextline(ifstream* reader, std::string* line, Parse* toparse)
{
   if(getline(*reader, *line))
   {
      toparse->string = line->c_str();
      toparse->size = line->size();

      return true;
   }
   return false;
}


inline Parse parse_whitespace(Parse toparse) 
{
	Parse outparse{toparse.string, toparse.size, 0};
	if (toparse.string[0] == ' ') {
		outparse.iToken = 1;
		outparse.string = &(toparse.string[1]);
		outparse.size = toparse.size - 1;
		outparse.hasToken = 1;
		return outparse;
	}
	outparse.iToken = 0;
	return outparse;
}

inline Parse parse_all_whitespace(Parse toparse) 
{
   do{
      toparse = parse_whitespace(toparse);
   } while(toparse.hasToken);
   return toparse;
}

inline Parse parse_letter(Parse toparse) 
{
	Parse outparse{ toparse.string, toparse.size, 0};
	if (isalpha(toparse.string[0])) {
		outparse.cToken = toparse.string[0];
		outparse.string = &(toparse.string[1]);
		outparse.size = toparse.size - 1;
		outparse.hasToken = 1;
		return outparse;
	}
	return outparse;
}

inline Parse parse_symbol(Parse toparse)
{
   Parse outparse{ toparse.string, toparse.size, 0};
	if (isgraph(toparse.string[0]) && !isalnum(toparse.string[0])) {
		outparse.cToken = toparse.string[0];
		outparse.string = &(toparse.string[1]);
		outparse.size = toparse.size - 1;
		outparse.hasToken = 1;
		return outparse;
	}
	return outparse;
}


// parses letters, digits or space character
inline Parse parse_name_char(Parse toparse)
{
 Parse outparse{ toparse.string, toparse.size, 0};
	if (isalnum(toparse.string[0]) || toparse.string[0] == ' ') {
		outparse.cToken = toparse.string[0];
		outparse.string = &(toparse.string[1]);
		outparse.size = toparse.size - 1;
		outparse.hasToken = 1;
		return outparse;
	}
	return outparse;
}


inline Parse parse_name(Parse toparse)
{
   char string_buffer[50];
   size_t sb_size = 0;
   Parse outparse{ toparse.string, toparse.size, 0};
   do{
      toparse = parse_name_char(toparse);
      if(toparse.hasToken) string_buffer[sb_size++] = toparse.cToken;
   } while(toparse.hasToken);
   string_buffer[sb_size] = '\0';
   strcpy(&outparse.string_buffer[0], &string_buffer[0]);
   return outparse;
}

// parses chars for tokens (without spaces)
inline Parse parse_token_char(Parse toparse)
{
 Parse outparse{ toparse.string, toparse.size, 0};
	if (isalnum(toparse.string[0]) || toparse.string[0] == '_' || toparse.string[0] == '.') {
		outparse.cToken = toparse.string[0];
		outparse.string = &(toparse.string[1]);
		outparse.size = toparse.size - 1;
		outparse.hasToken = 1;
		return outparse;
	}
	return outparse;
}


inline Parse parse_token(Parse toparse)
{
   char string_buffer[50];
   size_t sb_size = 0;
	do{
      toparse = parse_token_char(toparse);
      if(toparse.hasToken) string_buffer[sb_size++] = toparse.cToken;
   } while(toparse.hasToken);
   string_buffer[sb_size] = '\0';
   strcpy(&toparse.string_buffer[0], &string_buffer[0]);
	return toparse;
}


inline Parse parse_int(Parse toparse) 
{
	Parse outparse{ toparse.string, toparse.size, 0};
	u32 ten_powers[10]{1, 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000, 1000000000};
	char int_buf[10];
	u16 count = 0;
	u16 rev_count = 0;
	u16 sign = 1;
	if (outparse.string[0] == '-') 
   {
		outparse.string = &(outparse.string[1]);
		outparse.size -= 1;
		sign = -1;
	}
	if (isdigit(outparse.string[0]))
   {
		do {
			int_buf[count++] = outparse.string[0];
			outparse.string = &(outparse.string[1]);
			outparse.size -= 1;
		} while (isdigit(outparse.string[0]));
		while (count > 0) 
      {
			outparse.iToken += (int_buf[count - 1] - '0') * ten_powers[rev_count];
			rev_count++;
			count--;
		}
		outparse.iToken *= sign;
		outparse.hasToken = 1;
	}
	return outparse;
}

inline Parse parse_uint(Parse toparse) 
{
	Parse outparse{ toparse.string, toparse.size, 0};
	u32 ten_powers[10]{1, 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000, 1000000000};
	char int_buf[10];
	u16 count = 0;
	u16 rev_count = 0;
	if (isdigit(outparse.string[0]))
   {
		do {
			int_buf[count++] = outparse.string[0];
			outparse.string = &(outparse.string[1]);
			outparse.size -= 1;
		} while (isdigit(outparse.string[0]));
		while (count > 0) 
      {
			outparse.uiToken += (int_buf[count - 1] - '0') * ten_powers[rev_count];
			rev_count++;
			count--;
		}
		outparse.hasToken = 1;
	}
	return outparse;
}


inline Parse parse_float(Parse toparse) 
{
	Parse outparse{ toparse.string, toparse.size, 0 };
	int ten_powers[10]{ 1, 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000, 1000000000 };
	float ten_inverse_powers[10]{ 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f,
		0.000001f, 0.0000001f, 0.00000001f, 0.000000001f, 0.0000000001f};
	char int_buf[10];
	char float_buf[10];
	int count = 0, fcount = 0;
	int rev_count = 0;
	int sign = 1;
	if (outparse.string[0] == '-') 
   {
		outparse.string = &(outparse.string[1]);
		outparse.size -= - 1;
		sign = -1;
	}
	if (isdigit(outparse.string[0])) 
   {
		do{
			int_buf[count++] = outparse.string[0];
			outparse.string = &(outparse.string[1]);
			outparse.size -= 1;
		} while (isdigit(outparse.string[0])); 

		if(outparse.string[0] == '.')
      {
			outparse.string = &(outparse.string[1]);
			outparse.size -= 1;
			while (isdigit(outparse.string[0])){
				float_buf[fcount++] = outparse.string[0];
				outparse.string = &(outparse.string[1]);
				outparse.size -= 1;
			}; 
		}
		while (count > 0) 
      {
			outparse.fToken += (int_buf[count - 1] - '0') * ten_powers[rev_count];
			rev_count++;
			count--;
		}
		rev_count = 0;
		while(fcount > 0)
      {
			outparse.fToken += (float_buf[rev_count] - '0') * ten_inverse_powers[rev_count];
			rev_count++;
			fcount--;
		}
		outparse.fToken *= sign;
		outparse.hasToken = 1;
	}
	return outparse;
}


inline Parse parse_float_vector(Parse p)
{
   float x, y, z;
   p = parse_all_whitespace(p);
   p = parse_float(p);
   x = p.fToken;

   p = parse_all_whitespace(p);
   p = parse_float(p);
   y = p.fToken;

   p = parse_all_whitespace(p);
   p = parse_float(p);
   z = p.fToken;

   p.vec3[0] = x;
   p.vec3[1] = y;
   p.vec3[2] = z;
   return p;
}