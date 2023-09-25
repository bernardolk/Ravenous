#include <engine/core/types.h>
#include <string>
#include <map>
#include <engine/render/Shader.h>
#include <engine/render/text/character.h>
#ifndef GLAD_INCL
#define GLAD_INCL
#include <glad/glad.h>
#endif
#include "engine/geometry/mesh.h"
#include <iostream>
#include <glm/gtx/normal.hpp>
#include "engine/core/logging.h"
#include <engine/rvn.h>
#include <engine/render/text/TextRenderer.h>

#include <ft2build.h>
#include FT_FREETYPE_H

map<string, gl_charmap> FontCatalogue;


void RenderText(float X, float Y, string Text)
{
	RenderText("consola12", X, Y, vec3{1.0, 1.0, 1.0}, 1.0, false, Text);
}

void RenderText(string Font, float X, float Y, string Text)
{
	RenderText(Font, X, Y, vec3{1.0, 1.0, 1.0}, 1.0, false, Text);
}

void RenderText(float X, float Y, vec3 Color, string Text)
{
	RenderText("consola12", X, Y, Color, 1.0, false, Text);
}

void RenderText(string Font, float X, float Y, bool Center, string Text)
{
	RenderText(Font, X, Y, vec3{1.0, 1.0, 1.0}, 1.0, Center, Text);
}

void RenderText(string Font, float X, float Y, vec3 Color, string Text)
{
	RenderText(Font, X, Y, Color, 1.0, false, Text);
}

void RenderText(string Font, float X, float Y, vec3 Color, bool Center, string Text)
{
	RenderText(Font, X, Y, Color, 1.0, Center, Text);
}

void RenderText(string Font, float X, float Y, float Scale, string Text)
{
	RenderText(Font, X, Y, vec3{1.0, 1.0, 1.0}, Scale, false, Text);
}

void RenderText(string Font, float X, float Y, vec3 Color, float Scale, string Text)
{
	RenderText(Font, X, Y, Color, Scale, false, Text);
}

void RenderText(string Font, float X, float Y, vec3 Color, float Scale, bool Center, string Text)
{
	// Finds text shader in catalogue and set variables 
	auto TextShader = ShaderCatalogue.find("text")->second;
	TextShader->Use();
	TextShader->SetFloat3("textColor", Color.x, Color.y, Color.z);

	// Finds text drawing geometry in geometry catalogue
	auto TextGeometry = GeometryCatalogue.find("text")->second;
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(TextGeometry->gl_data.VAO);

	// Try finding font in catalogue, if doesn't find, tries loading it
	gl_charmap Charmap;
	auto FontQuery = FontCatalogue.find(Font);
	if (FontQuery == FontCatalogue.end())
	{
		// search for font size in font name (e.g. Consola12) and loads it
		int Index = 0;
		while (true)
		{
			if (!isalpha(Font[Index]))
				break;
			if (Index + 1 == Font.size())
			{
				print("Font '%s' could not be loaded because no size was appended to its name in render_text function call.", Font.c_str());
				return;
			}

			Index++;
		}

		string SizeStr = Font.substr(Index, Font.size());
		string FontFilename = Font.substr(0, Index) + ".ttf";

		int FontSize = std::stoi(SizeStr);
		Charmap = LoadTextTextures(FontFilename, FontSize);
	}
	else
	{
		Charmap = FontQuery->second;
	}

	//@todo add enum to CENTER, LEFT ALIGN (default, no extra work) and RIGHT ALIGN
	if (Center)
	{
		std::string::iterator It;
		float XSum = 0;
		for (It = Text.begin(); It != Text.end(); ++It)
		{
			auto Ch = Charmap[*It];
			XSum += Ch.bearing.x * Scale + Ch.size.x * Scale;
		}
		X -= XSum / 2.0;
	}


	glDepthFunc(GL_ALWAYS);
	std::string::iterator CharIt;
	for (CharIt = Text.begin(); CharIt != Text.end(); ++CharIt)
	{
		RCharacter Ch = Charmap[*CharIt];

		GLfloat Xpos = X + Ch.Bearing.x * Scale;
		GLfloat Ypos = Y - (Ch.Size.y - Ch.Bearing.y) * Scale;
		GLfloat W = Ch.Size.x * Scale;
		GLfloat H = Ch.Size.y * Scale;
		// Update VBO for each character
		GLfloat Vertices[6][4] = {
		{Xpos, Ypos + H, 0.0, 0.0},
		{Xpos, Ypos, 0.0, 1.0},
		{Xpos + W, Ypos, 1.0, 1.0},
		{Xpos, Ypos + H, 0.0, 0.0},
		{Xpos + W, Ypos, 1.0, 1.0},
		{Xpos + W, Ypos + H, 1.0, 0.0}
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, Ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, TextGeometry->gl_data.VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		X += (Ch.Advance >> 6) * Scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glDepthFunc(GL_LESS);
}

gl_charmap LoadTextTextures(string Font, int Size)
{
	// Load font
	FT_Library Ft;
	if (FT_Init_FreeType(&Ft))
		fatal_error("Freetype: Could not init FreeType Library");

	FT_Face Face;
	string Filepath = Paths::Fonts + Font;
	if (FT_New_Face(Ft, Filepath.c_str(), 0, &Face))
		Log(LOG_ERROR, "Freetype: Failed to load font");

	FT_Set_Pixel_Sizes(Face, 0, Size);

	gl_charmap FontCharmap;
	//we will store all characters inside the Characters map
	for (GLubyte C = 0; C < 128; C++)
	{
		//Load character glyph
		if (FT_Load_Char(Face, C, FT_LOAD_RENDER))
		{
			Log(LOG_ERROR, "Freetype: Failed to load Glyph");
			continue;
		}

		GLuint GylphTexture;
		glGenTextures(1, &GylphTexture);
		glBindTexture(GL_TEXTURE_2D, GylphTexture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			Face->glyph->bitmap.width,
			Face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			Face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		RCharacter Character = {
			.TextureID = GylphTexture,
			.Advance = static_cast<uint>(Face->glyph->advance.x),
			.Size = glm::ivec2(Face->glyph->bitmap.width, Face->glyph->bitmap.rows),
			.Bearing = glm::ivec2(Face->glyph->bitmap_left, Face->glyph->bitmap_top)
		};

		FontCharmap.insert(std::pair<GLchar, RCharacter>(C, Character));
	}

	// saves font chars to catalogue
	auto Separator = Font.find('.');
	string FontName = Font.substr(0, Separator);
	string FontCatalogueName = FontName + std::to_string(Size);

	FontCatalogue.insert({FontCatalogueName, FontCharmap});

	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(Face);
	FT_Done_FreeType(Ft);

	return FontCharmap;
}
