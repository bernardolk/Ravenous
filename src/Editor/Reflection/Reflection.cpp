#include "Reflection.h"

template<>
int Reflection::FromString<int>(string& Value)
{
	return std::stoi(Value);
}

template<>
float Reflection::FromString<float>(string& Value)
{
	return std::stof(Value);
}

template<>
double Reflection::FromString<double>(string& Value)
{
	return std::stod(Value);
}

template<>
long double Reflection::FromString<long double>(string& Value)
{
	return std::stold(Value);
}

template<>
bool Reflection::FromString<bool>(string& Value)
{
	return Value == "true" || Value == "True" || Value == "TRUE";
}

template<>
char Reflection::FromString<char>(string& Value)
{
	return Value[0];
}

template<>
string Reflection::FromString<string>(string& Value)
{
	if (Value[0] == '"')
	{
		Value.erase(0, 1);
		Value.erase(Value.size() - 1);
	}
	return Value;
}

string Reflection::ToString(string& Field)
{
	if (Field.empty())
		return "\"\"";

	return "\"" + Field + "\"";
};

string Reflection::ToString(char& Field)
{
	return string(1, Field);
};

string Reflection::ToString(bool& Field)
{
	if (Field)
		return "true";
	return "false";
};

void Reflection::ParseObject(string& Data, map<string, string>& FieldValueMap, bool IncludeHeader)
{
	string ToParse = Data;
	string _;

	// discard header if parsing nested object
	GetLine(ToParse, '{');

	if (IncludeHeader)
	{
		GetLine(ToParse, '{');
	}

	while (true)
	{
		string FieldData = GetLine(ToParse, ',');
		if (FieldData == "")
			break;

		GetLine(FieldData, ' ');
		string FieldNameToken = GetLine(FieldData, ' ');

		// stop processing fields
		if (FieldNameToken[0] == '}')
			continue;

		GetLine(FieldData, ' ');
		string FieldTypeToken = GetLine(FieldData, ' ');
		GetLine(FieldData, ' ');
		string FieldValue = GetLine(FieldData, ' ');

		// trim
		if (FieldNameToken.size() > 2)
		{
			FieldNameToken.erase(0, 1);
			FieldNameToken.erase(FieldNameToken.size() - 1);
		}

		// process nested object if field is a nested object
		if (FieldValue[0] == '{')
		{
			sprintf(&FieldValue[0], "{%s}", GetLine(ToParse, '}').c_str());
		}

		FieldValueMap[FieldNameToken] = FieldValue;
	}
}
