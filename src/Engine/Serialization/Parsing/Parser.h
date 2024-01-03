#pragma once
#include "Engine/Core/Core.h"
#include <sstream>

static constexpr uint StrBufferSize = 3000;
static constexpr uint NameStringBufferSize = 300;
static constexpr uint TokenStringBufferSize = 300;
static constexpr uint LineStringBufferSize = 3000;

struct ParseUnit
{
	string String;
	uint Size = 0;
	uint8 HasToken = 0;

	union
	{
		int IToken;
		float FToken;
		char CToken;
		uint UiToken;
		uint64 U64Token;
		float Vec3[3];
		float Vec2[2];
	};

	void AdvanceChar()
	{
		String = &(String[1]);
		Size--;
	}
};

// ====================================================================
//	Parser
//		Text file parsing utility. Used for .ref, .obj and .txt files.
// ====================================================================
struct Parser
{
	int LineCount = 0;
	//std::ifstream Reader;
	std::istringstream Stream;
	FILE* InputFile = nullptr;
	ParseUnit P{};

	char* ContentBuffer = nullptr;
	uint ContentSize = 0;
	uint Offset = 0;

	static inline char ParsingStringBuffer[StrBufferSize]{};
	
	bool bReaderSet = false;

	explicit Parser(const string& Filepath);

	Parser(const string& TextBuffer, const int BufferSize)
	{
		this->P.String = TextBuffer.c_str();
		this->P.Size = BufferSize;

		if (StrBufferSize < BufferSize) {
			FatalError("Incoming string is greater than reserved buffer for Parsers.")
		}
	}

	~Parser()
	{
		if (bReaderSet && ContentBuffer)
		{
			free(ContentBuffer);
		}
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
	void ParseChar();
	void ParseInt();
	void ParseUint();
	void ParseU64();
	void ParseFloat();
	void ParseVec3();
	void ParseVec2();
	void ParseNewLine();
	void ParseLine();
	void ParseQuote();
	void ParseFieldValueToken();
	void ParseFieldTypeToken();

	void ClearParseBuffer();
	bool HasToken() const;

	constexpr static uint TenPowers[10]
	{
		1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
	};

	constexpr static float TenInversePowers[10]
	{
		0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f, 0.00000001f, 0.000000001f, 0.0000000001f
	};
};

template<typename T>
T GetParsed(Parser& Parser)
{
	return *reinterpret_cast<T*>(&Parser.P.IToken);
}

template<>
inline string GetParsed(Parser& Parser)
{
	return Parser.ParsingStringBuffer;
}

template<>
inline glm::vec3 GetParsed(Parser& Parser)
{
	if (Parser.P.HasToken == 0)
		FatalError("FATAL: Parse has no vec3 value to be retrieved. Check line being parsed.");

	return glm::vec3{Parser.P.Vec3[0], Parser.P.Vec3[1], Parser.P.Vec3[2]};
}

template<>
inline glm::vec2 GetParsed(Parser& Parser)
{
	if (Parser.P.HasToken == 0)
		FatalError("FATAL: Parse has no vec3 value to be retrieved. Check line being parsed.");

	return glm::vec2{Parser.P.Vec2[0], Parser.P.Vec2[1]};
}
