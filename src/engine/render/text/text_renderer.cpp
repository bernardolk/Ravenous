#include <engine/core/types.h>
#include <string>
#include <map>
#include <engine/render/shader.h>
#include <engine/render/text/character.h>
#ifndef GLAD_INCL
#define GLAD_INCL
#include <glad/glad.h>
#endif
#include <vector>
#include <engine/mesh.h>
#include <iostream>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/triangle.h>
#include <glm/gtx/rotate_vector.hpp>
#include <engine/render/text/face.h>
#include <engine/logging.h>
#include <engine/rvn.h>
#include <engine/render/text/text_renderer.h>

#include <ft2build.h>
#include FT_FREETYPE_H

std::map<std::string, gl_charmap> Font_Catalogue;


void render_text(float x, float y, std::string text)
{
	render_text("consola12", x, y, vec3{1.0, 1.0, 1.0}, 1.0, false, text);
}

void render_text(std::string font, float x, float y, std::string text)
{
	render_text(font, x, y, vec3{1.0, 1.0, 1.0}, 1.0, false, text);
}

void render_text(float x, float y, vec3 color, std::string text)
{
	render_text("consola12", x, y, color, 1.0, false, text);
}

void render_text(std::string font, float x, float y, bool center, std::string text)
{
	render_text(font, x, y, vec3{1.0, 1.0, 1.0}, 1.0, center, text);
}

void render_text(std::string font, float x, float y, vec3 color, std::string text)
{
	render_text(font, x, y, color, 1.0, false, text);
}

void render_text(std::string font, float x, float y, vec3 color, bool center, std::string text)
{
	render_text(font, x, y, color, 1.0, center, text);
}

void render_text(std::string font, float x, float y, float scale, std::string text)
{
	render_text(font, x, y, vec3{1.0, 1.0, 1.0}, scale, false, text);
}

void render_text(std::string font, float x, float y, vec3 color, float scale, std::string text)
{
	render_text(font, x, y, color, scale, false, text);
}

void render_text(std::string font, float x, float y, vec3 color, float scale, bool center, std::string text)
{
	// Finds text shader in catalogue and set variables 
	auto text_shader = ShaderCatalogue.find("text")->second;
	text_shader->Use();
	text_shader->SetFloat3("textColor", color.x, color.y, color.z);

	// Finds text drawing geometry in geometry catalogue
	auto text_geometry = GeometryCatalogue.find("text")->second;
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(text_geometry->gl_data.VAO);

	// Try finding font in catalogue, if doesn't find, tries loading it
	gl_charmap charmap;
	auto font_query = Font_Catalogue.find(font);
	if(font_query == Font_Catalogue.end())
	{
		// search for font size in font name (e.g. Consola12) and loads it
		int ind = 0;
		while(true)
		{
			if(!isalpha(font[ind]))
				break;
			if(ind + 1 == font.size())
			{
				std::cout << "Font '" << font << "' could not be loaded because no size was "
				<< "appended to its name in render_text function call.";
				return;
			}

			ind++;
		}

		std::string size_str = font.substr(ind, font.size());
		std::string font_filename = font.substr(0, ind) + ".ttf";

		int font_size = std::stoi(size_str);
		charmap = load_text_textures(font_filename, font_size);
	}
	else
	{
		charmap = font_query->second;
	}

	//@todo add enum to CENTER, LEFT ALIGN (default, no extra work) and RIGHT ALIGN
	if(center)
	{
		std::string::iterator it;
		float x_sum = 0;
		for(it = text.begin(); it != text.end(); ++it)
		{
			auto ch = charmap[*it];
			x_sum += ch.bearing.x * scale + ch.size.x * scale;
		}
		x -= x_sum / 2.0;
	}


	glDepthFunc(GL_ALWAYS);
	std::string::iterator c;
	for(c = text.begin(); c != text.end(); ++c)
	{
		Character ch = charmap[*c];

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;
		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
		{xpos, ypos + h, 0.0, 0.0},
		{xpos, ypos, 0.0, 1.0},
		{xpos + w, ypos, 1.0, 1.0},
		{xpos, ypos + h, 0.0, 0.0},
		{xpos + w, ypos, 1.0, 1.0},
		{xpos + w, ypos + h, 1.0, 0.0}
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.texture_id);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, text_geometry->gl_data.VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glDepthFunc(GL_LESS);
}

gl_charmap load_text_textures(std::string font, int size)
{
	// Load font
	FT_Library ft;
	if(FT_Init_FreeType(&ft))
		Quit_fatal("Freetype: Could not init FreeType Library");

	FT_Face face;
	std::string filepath = Paths::Fonts + font;
	if(FT_New_Face(ft, filepath.c_str(), 0, &face))
		log(LOG_ERROR, "Freetype: Failed to load font");

	FT_Set_Pixel_Sizes(face, 0, size);

	gl_charmap font_charmap;
	//we will store all characters inside the Characters map
	for(GLubyte c = 0; c < 128; c++)
	{
		//Load character glyph
		if(FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			log(LOG_ERROR, "Freetype: Failed to load Glyph");
			continue;
		}

		GLuint gylphTexture;
		glGenTextures(1, &gylphTexture);
		glBindTexture(GL_TEXTURE_2D, gylphTexture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
		.texture_id = gylphTexture,
		.advance = static_cast<u32>(face->glyph->advance.x),
		.size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top)
		};

		font_charmap.insert(std::pair<GLchar, Character>(c, character));
	}

	// saves font chars to catalogue
	auto separator = font.find('.');
	std::string font_name = font.substr(0, separator);
	std::string font_catalogue_name = font_name + std::to_string(size);

	Font_Catalogue.insert({font_catalogue_name, font_charmap});

	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return font_charmap;
}
