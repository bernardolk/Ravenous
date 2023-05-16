#include "Reflection.h"

template<>
int Reflection::FromString<int>(string& value)
{
	return std::stoi(value);
}

template<>
float Reflection::FromString<float>(string& value)
{
	return std::stof(value);
}

template<>
double Reflection::FromString<double>(string& value)
{
	return std::stod(value);
}

template<>
long double Reflection::FromString<long double>(string& value)
{
	return std::stold(value);
}

template<>
bool Reflection::FromString<bool>(string& value)
{
	if(value == "true") return true;
	else return false;
}

template<>
char Reflection::FromString<char>(string& value)
{
	return value[0];
}

template<>
string Reflection::FromString<string>(string& value)
{
	if(value[0] == '"')
	{
		value.erase(0, 1);
		value.erase(value.size() - 1);
	}
	return value;
}

string Reflection::ToString(string& field)
{
	if (field.empty())
		return "\"\"";

	return "\"" + field + "\"";
};

string Reflection::ToString(char& field)
{
	return string(1, field);
};

string Reflection::ToString(bool& field)
{
	if (field) return "true";
	return "false";
};

void Reflection::ParseObject(string& data, map<string, string>& field_value_map, bool include_header)
{
	string to_parse = data;
	string _;

	// discard header if parsing nested object
	GetLine(to_parse, '{');

	if (include_header)
	{
		GetLine(to_parse, '{');
	}

	while (true)
	{
		string field_data = GetLine(to_parse, ',');
		if(field_data == "")
			break;

		GetLine(field_data, ' ');
		string field_name_token = GetLine(field_data, ' ');

		// stop processing fields
		if(field_name_token[0] == '}')
			continue;

		GetLine(field_data, ' ');
		string field_type_token = GetLine(field_data, ' ');
		GetLine(field_data, ' ');
		string field_value = GetLine(field_data, ' ');

		// trim
		if(field_name_token.size() > 2)
		{
			field_name_token.erase(0, 1);
			field_name_token.erase(field_name_token.size() - 1);
		}

		// process nested object if field is a nested object
		if(field_value[0] == '{')
		{
			sprintf(&field_value[0], "{%s}", GetLine(to_parse, '}').c_str());
		}

		field_value_map[field_name_token] = field_value;
	}
}
