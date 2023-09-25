#include <string>
#include <fstream>
#include <sstream>
#include <engine/core/types.h>
#include <engine/serialization/parsing/parser.h>

bool Parser::NextLine()
{
	if (getline(Reader, P.String))
	{
		P.Size = P.String.size();
		LineCount++;
		return true;
	}

	Reader.close();
	return false;
}

bool Parser::HasToken() const
{
	return P.HasToken;
}


void Parser::ParseWhitespace()
{
	ClearParseBuffer();

	if (P.String[0] == ' ')
	{
		P.IToken = 1;
		P.AdvanceChar();
		P.HasToken = 1;
	}
}

void Parser::ParseAllWhitespace()
{
	ClearParseBuffer();

	do
	{
		ParseWhitespace();
	} while (P.HasToken);
}

void Parser::ParseLetter()
{
	ClearParseBuffer();

	if (isalpha(P.String[0]))
	{
		P.CToken = P.String[0];
		P.AdvanceChar();
		P.HasToken = 1;
	}
}

void Parser::ParseSymbol()
{
	ClearParseBuffer();

	const auto Char = P.String[0];
	if (isgraph(Char) && !isalnum(Char) && Char != ' ')
	{
		P.CToken = Char;
		P.AdvanceChar();
		P.HasToken = 1;
	}
}

void Parser::ParseNameChar()
{
	// parses letters, digits, space or underline character
	ClearParseBuffer();

	const auto Char = P.String[0];
	if (isalnum(Char) || Char == ' ' || Char == '_')
	{
		P.CToken = Char;
		P.AdvanceChar();
		P.HasToken = 1;
	}
}

void Parser::ParseName()
{
	// Names consists of alphanumeric or space chars
	ClearParseBuffer();

	char StringBuffer[50];
	uint BufferSize = 0;
	do
	{
		ParseNameChar();
		if (P.HasToken)
			StringBuffer[BufferSize++] = P.CToken;
	} while (P.HasToken);

	StringBuffer[BufferSize] = '\0';
	if (BufferSize > 0)
		P.HasToken = 1;
	strcpy_s(P.StringBuffer, &StringBuffer[0]);
}

void Parser::ParseTokenChar()
{
	// parses chars for tokens (without spaces)
	ClearParseBuffer();

	const auto Char = P.String[0];
	if (isalnum(Char) || Char == '_' || Char == '.')
	{
		P.CToken = P.String[0];
		P.AdvanceChar();
		P.HasToken = 1;
	}
}

void Parser::ParseToken()
{
	// Tokens consists of alphanumeric or '_' or '.' chars
	ClearParseBuffer();

	char StringBuffer[50];
	uint BufferSize = 0;
	do
	{
		ParseTokenChar();
		if (P.HasToken)
			StringBuffer[BufferSize++] = P.CToken;
	} while (P.HasToken);

	StringBuffer[BufferSize] = '\0';
	if (BufferSize > 0)
		P.HasToken = 1;
	strcpy_s(P.StringBuffer, &StringBuffer[0]);
}

void Parser::ParseInt()
{
	ClearParseBuffer();

	uint16 Sign = 1;
	if (P.String[0] == '-')
	{
		P.AdvanceChar();
		Sign = -1;
	}
	if (isdigit(P.String[0]))
	{
		uint16 Count = 0;
		char IntBuf[10];

		do
		{
			IntBuf[Count++] = P.String[0];
			P.AdvanceChar();
		} while (isdigit(P.String[0]));

		for (int i = 0; i < Count; i++)
			P.IToken += (IntBuf[Count - (1 + i)] - '0') * TenPowers[i];

		P.IToken *= Sign;
		P.HasToken = 1;
	}
}

void Parser::ParseUint()
{
	ClearParseBuffer();

	if (isdigit(P.String[0]))
	{
		uint16 Count = 0;
		char IntBuf[10];
		do
		{
			IntBuf[Count++] = P.String[0];
			P.AdvanceChar();
		} while (isdigit(P.String[0]));

		for (int i = 0; i < Count; i++)
			P.UiToken += (IntBuf[Count - (1 + i)] - '0') * TenPowers[i];

		P.HasToken = 1;
	}
}

void Parser::ParseU64()
{
	ClearParseBuffer();

	if (isdigit(P.String[0]))
	{
		uint16 Count = 0;
		char IntBuf[15];
		do
		{
			IntBuf[Count++] = P.String[0];
			P.AdvanceChar();
		} while (isdigit(P.String[0]) && Count < 15);

		for (int i = 0; i < Count; i++)
			P.U64Token += (IntBuf[Count - (1 + i)] - '0') * TenPowers[i];

		P.HasToken = 1;
	}
}

void Parser::ParseFloat()
{
	ClearParseBuffer();

	char IntBuf[10]{};
	float Sign = 1.f;

	if (P.String[0] == '-')
	{
		P.AdvanceChar();
		Sign = -1.f;
	}

	int Count = 0;
	int FCount = 0;
	char FloatBuf[10];
	while (isdigit(P.String[0]))
	{
		IntBuf[Count++] = P.String[0];
		P.AdvanceChar();
	}

	if (P.String[0] == '.')
	{
		P.AdvanceChar();
		while (isdigit(P.String[0]))
		{
			FloatBuf[FCount++] = P.String[0];
			P.AdvanceChar();
		}
	}

	//@TODO: We are losing precision here. Investigate at some point.
	for (int i = 0; i < Count; i ++)
		P.FToken += (IntBuf[Count - (1 + i)] - '0') * TenPowers[i];

	for (int i = 0; i < FCount; i ++)
		P.FToken += (FloatBuf[i] - '0') * TenInversePowers[i];

	P.FToken *= Sign;
	P.HasToken = 1;

}

void Parser::ParseVec3()
{
	ClearParseBuffer();

	ParseAllWhitespace();
	ParseFloat();
	if (!P.HasToken)
		return;
	const float X = P.FToken;

	ParseAllWhitespace();
	ParseFloat();
	if (!P.HasToken)
		return;
	const float Y = P.FToken;

	ParseAllWhitespace();
	ParseFloat();
	if (!P.HasToken)
		return;
	const float Z = P.FToken;

	P.Vec3[0] = X;
	P.Vec3[1] = Y;
	P.Vec3[2] = Z;
}

void Parser::ParseVec2()
{
	ClearParseBuffer();

	ParseAllWhitespace();
	ParseFloat();
	if (!P.HasToken)
		return;
	const float U = P.FToken;

	ParseAllWhitespace();
	ParseFloat();
	if (!P.HasToken)
		return;
	const float V = P.FToken;

	P.Vec2[0] = U;
	P.Vec2[1] = V;
}

void Parser::ClearParseBuffer()
{
	P = ParseUnit{.String = P.String, .Size = P.Size};
}
