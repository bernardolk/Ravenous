#include "Reflection.h"

#include "engine/serialization/parsing/parser.h"

// @TODO: FromString should be a general Parsing library feature and not reflection-specific.
// @TODO: ToString also sounds like a feature we would want to add to our String library in the future (we don't have one yet).

#define ToStringConstVersion(Type) \
	string Reflection::ToString(const Type& Field) \
	{ \
		return ToStringRemoveConst<Type>(Field); \
	}

string Reflection::ToString(string& Field)
{
	if (Field.empty())
		return "\"\"";

	return "\"" + Field + "\"";
}
//ToStringConstVersion(string);

string Reflection::ToString(char& Field)
{
	return string(1, Field);
}
//ToStringConstVersion(char);


string Reflection::ToString(bool& Field)
{
	if (Field)
		return "true";
	return "false";
}
//ToStringConstVersion(bool);

string Reflection::ToString(vec3& Field)
{
	return "{" + std::to_string(Field.x) + " " + std::to_string(Field.y) + " " + std::to_string(Field.z) + "}";
}
//ToStringConstVersion(vec3);

// ------------------------------------------------------------------

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

// template<>
// vec3 Reflection::FromString<vec3>(string& Value)
// {
// 	Parser Parse{Value, Value.size()};
// 	// discard '{' token
// 	Parse.ParseTokenChar();
// 	Parse.ParseVec3();
// 	return GetParsed<vec3>(Parse);
// }

// ------------------------------------------------------------------

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
		// @TODO: Revise why we don't put the code below in an else after the last if
		GetLine(FieldData, ' ');
		string FieldValue;
		FieldValue.reserve(100);
		FieldValue = GetLine(FieldData, ' ');

		// trim
		if (FieldNameToken.size() > 2)
		{
			FieldNameToken.erase(0, 1);
			FieldNameToken.erase(FieldNameToken.size() - 1);
		}

		// process nested object if field is a nested object
		if (FieldValue[0] == '{')
		{
			string NestedObject = GetLine(ToParse, '}');
			int BytesWritten = sprintf_s(&FieldValue[0], 100, "{%s}", NestedObject.c_str());
			if (BytesWritten == -1) {
				fatal_error("Nested object procesing failed.");
			}
		}

		FieldValueMap[FieldNameToken] = FieldValue;
	}
}
