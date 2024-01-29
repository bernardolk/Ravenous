#include "Reflection.h"

#include <iomanip>
#include <sstream>

#include "Serialization.h"
#include "engine/catalogues.h"
#include "Engine/Collision/CollisionMesh.h"
#include "Engine/Geometry/Cylinder.h"
#include "Engine/Render/Shader.h"
#include "Engine/Geometry/Mesh.h"
#include "engine/serialization/parsing/parser.h"

// @TODO: FromString should be a general Parsing library feature and not reflection-specific.
// @TODO: ToString also sounds like a feature we would want to add to our String library in the future (we don't have one yet).
// @TODO: Currently, this library does not support nested reflected object serialization. This means that any data that needs to be reflected / serialized needs to be flat out inline in the struct definition.
//		Inheriting reflected fields works though.

/* ====================================
 * ToString specializations
 * ==================================== */
string Reflection::ToString(string& Field)
{
	if (Field.empty())
		return "\"\"";

	return "\"" + Field + "\"";
}

string Reflection::ToString(char& Field)
{
	return string(1, Field);
}

string Reflection::ToString(bool& Field)
{
	if (Field) {
		return "true";
	}
	return "false";
}

string Reflection::ToString(vec3& Field)
{
	return std::to_string(Field.x) + " " + std::to_string(Field.y) + " " + std::to_string(Field.z);
}

string Reflection::ToString(RUUID& Field)
{
	/* Examples:
	 *  11111888887777766666ULL ->  "11111-88888-77777-66666"
	 *  888887777766666ULL ->		"00000-88888-77777-66666"
	 */
	
	string Result = "\"";
	// Convert the uint64_t value to a string with leading zeros and a total width of 20.
	std::ostringstream StringStream;
	StringStream << std::setw(20) << std::setfill('0') << Field;
	string PaddedString = StringStream.str();

	// Create a formatted string with hyphens.
	string FormattedString;
	for (int i = 0; i < 20; ++i) {
		FormattedString += PaddedString[i];
		if ((i + 1) % 5 == 0 && i < 19) {
			FormattedString += '-';
		}
	}

	Result.append(FormattedString);
	Result.push_back('\"');

	return Result;
}

string Reflection::ToString(RShader* Field)
{
	if (!Field) {
		Break("Field was nullptr")
		return "nullptr";
	}

	return Field->Name;
}

string Reflection::ToString(RMesh* Field)
{
	if (!Field) {
		Break("Field was nullptr")
		return "nullptr";
	}

	return Field->Name;
}

string Reflection::ToString(RCollisionMesh* Field)
{
	if (!Field) {
		Break("Field was nullptr")
		return "nullptr";
	}

	return Field->Name;
}

string Reflection::ToString(RTexture Field)
{
	if (Field.Name.empty())
	{
		return "null";
	}
	
	return Field.Name;
}

string Reflection::ToString(RCylinder& Field)
{
	return ToString(Field.Position) + " " +  ToString(Field.Radius) + " " + ToString(Field.Height); 
}

/* ====================================
 * FromString specializations
 * ==================================== */

template<>
int Reflection::FromString<int>(const string& StringValue)
{
	return std::stoi(StringValue);
}

template<>
int64 Reflection::FromString<int64>(const string& StringValue)
{
	return std::stol(StringValue);
}

template<>
float Reflection::FromString<float>(const string& StringValue)
{
	return std::stof(StringValue);
}

template<>
double Reflection::FromString<double>(const string& StringValue)
{
	return std::stod(StringValue);
}

template<>
long double Reflection::FromString<long double>(const string& StringValue)
{
	return std::stold(StringValue);
}

template<>
bool Reflection::FromString<bool>(const string& StringValue)
{
	return StringValue == "true" || StringValue == "True" || StringValue == "TRUE";
}

template<>
char Reflection::FromString<char>(const string& StringValue)
{
	if (StringValue.size() > 0) {
		return StringValue[0];
	}

	return {};
}

template<>
string Reflection::FromString<string>(const string& StringValue)
{
	// remove leading quotes
	string ReturnString = StringValue;
	if (ReturnString[0] == '"')
	{
		ReturnString.erase(0, 1);
		ReturnString.erase(ReturnString.size() - 1);
	}
	return ReturnString;
}

template<>
vec3 Reflection::FromString<vec3>(const string& Value)
{
	Parser p{Value, (int)Value.size()};
	p.ParseVec3();
	return GetParsed<vec3>(p);
}

template<>
RUUID Reflection::FromString<RUUID>(const string& Value)
{
	// Remove hyphens from the formatted string
	string stringValue;
	for (char c : Value) {
		if (c != '-') {
			stringValue += c;
		}
	}

	// Convert the string to uint64
	std::istringstream StringStream(stringValue);
	uint64 Result = 0;
	StringStream >> Result;
	return {Result};
}

template<>
RShader* Reflection::FromString<RShader*>(const string& Value)
{
	return GetShader(Value);
}

template<>
RMesh* Reflection::FromString<RMesh*>(const string& Value)
{
	return GetOrLoadMesh(Value);
}

template<>
RCollisionMesh* Reflection::FromString<RCollisionMesh*>(const string& Value)
{
	return GetOrLoadCollisionMesh(Value);
}

template<>
RTexture Reflection::FromString<RTexture>(const string& Value)
{
	return GetOrLoadTexture(Value);
}

template<>
RCylinder Reflection::FromString<RCylinder>(const string& Value)
{
	RCylinder Cylinder;
	
	Parser p{Value, (int)Value.size()};
	p.ParseVec3();
	Cylinder.Position = GetParsed<vec3>(p);
	p.ParseAllWhitespace();
	p.ParseFloat();
	Cylinder.Radius = GetParsed<float>(p);
	p.ParseAllWhitespace();
	p.ParseFloat();
	Cylinder.Height = GetParsed<float>(p);

	return Cylinder;
}

/* ====================================
 * LoadFromString
 * ==================================== */
EEntity* Reflection::LoadFromString(const string& SeralizedEntity)
{
	// Parse entity type from string contents
	Parser p(SeralizedEntity, SeralizedEntity.size());
	p.ParseNewLine();
	p.ParseToken();
	p.ParseAllWhitespace();
	p.ParseChar();
	p.ParseAllWhitespace();
	p.ParseToken();
	if (!p.HasToken()) {
		Break("Couldn't parse entity type from string");
	}
	const auto Type = GetParsed<string>(p);

	auto* Metadata = TypeMetadataManager::Get()->FindTypeMetadataByName(Type);
	if (!Metadata) {
		Break("Couldn't find metadata for Type \"%s\"", Type.c_str());
	}

	return Metadata->TypeInitFunction(SeralizedEntity);
}

/* ====================================
 * ParseFieldsFromSerializedObject
 * ==================================== */
void Reflection::ParseFieldsFromSerializedObject(const string& Data, map<string, string>& OutFieldValueMap)
{
	// ===============================
	// Grammar reference:
	// "%field : %type = %value\n"
	// ===============================
		
	string ToParse = Data;
	Parser p(ToParse, ToParse.size());
	
	while (true)
	{
		string Field;
		string Value;
		
		p.ParseAllWhitespace();
		p.ParseToken();
		if (!p.HasToken()) {
			break;
		}
		Field = GetParsed<string>(p);
		p.ParseAllWhitespace();
		p.ParseSymbol();
		p.ParseAllWhitespace();
		p.ParseFieldTypeToken();
		// PS: the field Type isnt used here for anything because the Setter function for that field is retrieved by the field name and it knows what type to use inside it. Neat.
		p.ParseAllWhitespace();
		p.ParseSymbol();
		p.ParseAllWhitespace();
		p.ParseQuote();
		p.ParseFieldValueToken();
		if (!p.HasToken()) {
			break;
		}
		Value = GetParsed<string>(p);
		p.ParseQuote();
		p.ParseAllWhitespace();
		p.ParseNewLine();
		
		OutFieldValueMap[Field] = Value;
	}	
}
