#pragma once
#include <fstream>

struct ParseUnit
{
	string String;
	uint Size = 0;
	uint8 HasToken = 0;
	union
	{
		char StringBuffer[50]{};
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

struct Parser
{
	int LineCount = 0;
	std::ifstream Reader;
	string Filepath;
	ParseUnit P{};

	explicit Parser(const string& Filepath)
	{
		this->Filepath = Filepath;
		this->Reader = std::ifstream(Filepath);
		if (!this->Reader.is_open())
			fatal_error("Couldn't open file '%s', path NOT FOUND", Filepath.c_str());
	}

	Parser(const string& TextBuffer, const int BufferSize)
	{
		this->P.String = TextBuffer.c_str();
		this->P.Size = BufferSize;
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

	constexpr static uint TenPowers[10]
	{
		1, 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000, 1000000000
	};

	constexpr static float TenInversePowers[10]
	{
		0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f,
		0.000001f, 0.0000001f, 0.00000001f, 0.000000001f, 0.0000000001f
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
	return Parser.P.StringBuffer;
}

template<>
inline glm::vec3 GetParsed(Parser& Parser)
{
	if (Parser.P.HasToken == 0)
		fatal_error("FATAL: Parse has no vec3 value to be retrieved. Check line being parsed.");

	return glm::vec3{Parser.P.Vec3[0], Parser.P.Vec3[1], Parser.P.Vec3[2]};
}

template<>
inline glm::vec2 GetParsed(Parser& Parser)
{
	if (Parser.P.HasToken == 0)
		fatal_error("FATAL: Parse has no vec3 value to be retrieved. Check line being parsed.");

	return glm::vec2{Parser.P.Vec2[0], Parser.P.Vec2[1]};
}
