#include <fstream>
#include <string>
#include <stdio.h>
#include <engine/core/types.h>
#include <engine/serialization/parsing/parser.h>

Parser::Parser(const string& Filepath)
{
	InputFile = fopen(Filepath.c_str(), "rb");
	if (InputFile == nullptr) {
		FatalError("Couldn't open file '%s', path NOT FOUND", Filepath.c_str());
	}

	fseek(InputFile, 0, SEEK_END);
	ContentSize = ftell(InputFile);
	fseek(InputFile, 0, SEEK_SET);

	ContentBuffer = (char*) malloc(ContentSize + 1);
	
	fread(ContentBuffer, 1, ContentSize, InputFile);
	fclose(InputFile);

	ContentBuffer[ContentSize] = '\0';

	Stream = std::istringstream(ContentBuffer);
	bReaderSet = true;
}

bool Parser::NextLine()
{
	if (!bReaderSet) FatalError("Parser was not initialized correctly.");
	
	if (getline(Stream, P.String))
	{
		P.Size = P.String.size();
		LineCount++;
		return true;
	}
	
	return false;
}

bool Parser::HasToken() const
{
	return P.HasToken;
}


void Parser::ParseWhitespace()
{
	ClearParseBuffer();

	auto& Char = P.String[0];
	if (Char == ' ' || Char == '\t')
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
	if (isgraph(Char) && !isalnum(Char) && Char != ' ' && Char != '\0')
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

	char StringBuffer[NameStringBufferSize];
	uint BufferSize = 0;
	do
	{
		ParseNameChar();
		if (P.HasToken)
			StringBuffer[BufferSize++] = P.CToken;
	} while (P.HasToken);

	StringBuffer[BufferSize] = '\0';
	if (BufferSize > 0) {
		P.HasToken = 1;
	}
	strcpy_s(ParsingStringBuffer, &StringBuffer[0]);
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

void Parser::ParseQuote()
{
	// parses chars for tokens (without spaces)
	ClearParseBuffer();

	const auto Char = P.String[0];
	if (Char == '\"')
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

	char StringBuffer[TokenStringBufferSize];
	uint BufferSize = 0;
	do
	{
		ParseTokenChar();
		if (P.HasToken)
			StringBuffer[BufferSize++] = P.CToken;
	} while (P.HasToken);

	// Set terminating character at last position
	StringBuffer[BufferSize] = '\0';

	if (BufferSize > 0) {
		P.HasToken = 1;
	}

	//@TODO @safety: Can this overflow ?
	strcpy_s(ParsingStringBuffer, &StringBuffer[0]);
}

void Parser::ParseFieldValueToken()
{
	ClearParseBuffer();

	char StringBuffer[TokenStringBufferSize];
	uint BufferSize = 0;
	do
	{
		ClearParseBuffer();

		const auto Char = P.String[0];
		if (isalnum(Char) || Char == '_' || Char == '.' || Char == '-' || Char == ' ')
		{
			P.CToken = P.String[0];
			P.AdvanceChar();
			P.HasToken = 1;
		}
		
		if (P.HasToken) {
			StringBuffer[BufferSize++] = P.CToken;
		}
	
	} while (P.HasToken);

	// Set terminating character at last position
	StringBuffer[BufferSize] = '\0';

	if (BufferSize > 0) {
		P.HasToken = 1;
	}

	//@TODO @safety: Can this overflow ?
	strcpy_s(ParsingStringBuffer, &StringBuffer[0]);
}

void Parser::ParseFieldTypeToken()
{
	ClearParseBuffer();

	char StringBuffer[TokenStringBufferSize];
	uint BufferSize = 0;
	do
	{
		ClearParseBuffer();

		const auto Char = P.String[0];
		if (isalnum(Char) || Char == '_' || Char == '*')
		{
			P.CToken = P.String[0];
			P.AdvanceChar();
			P.HasToken = 1;
		}
		
		if (P.HasToken) {
			StringBuffer[BufferSize++] = P.CToken;
		}
	
	} while (P.HasToken);

	// Set terminating character at last position
	StringBuffer[BufferSize] = '\0';

	if (BufferSize > 0) {
		P.HasToken = 1;
	}

	//@TODO @safety: Can this overflow ?
	strcpy_s(ParsingStringBuffer, &StringBuffer[0]);
}

void Parser::ParseChar()
{
	ClearParseBuffer();

	const auto Char = P.String[0];
	if (Char != '\0')
	{
		P.CToken = P.String[0];
		P.AdvanceChar();
		P.HasToken = 1;
	}
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

		for (int i = 0; i < Count; i++) {
			P.UiToken += (IntBuf[Count - (1 + i)] - '0') * TenPowers[i];
		}

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

		for (int i = 0; i < Count; i++) {
			P.U64Token += (IntBuf[Count - (1 + i)] - '0') * TenPowers[i];
		}

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

void Parser::ParseNewLine()
{
	ClearParseBuffer();

	if (P.String[0] == '\n')
	{
		P.CToken = '\n';
		P.AdvanceChar();
		P.HasToken = 1;
	}
}

void Parser::ParseLine()
{
	ClearParseBuffer();

	char StringBuffer[LineStringBufferSize];
	uint BufferSize = 0;
	
	while (true)
	{
		ParseChar();
		if (!P.HasToken || P.CToken == '\n') 
			break;
		
		StringBuffer[BufferSize++] = P.CToken;
	}

	// Set terminating character at last position
	StringBuffer[BufferSize] = '\0';

	if (BufferSize > 0) {
		P.HasToken = 1;
	}

	//@TODO @safety: Can this overflow ?
	strcpy_s(ParsingStringBuffer, &StringBuffer[0]);
}

void Parser::ClearParseBuffer()
{
	P.HasToken = 0;
	P.IToken = 0;
}
