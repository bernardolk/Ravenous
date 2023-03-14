#pragma once

struct Character;

using gl_charmap = std::map<char, Character>;

void RenderText(float x, float y, std::string text);
void RenderText(float x, float y, vec3 color, std::string text);
void RenderText(std::string font, float x, float y, std::string text);
void RenderText(std::string font, float x, float y, bool center, std::string text);
auto RenderText(std::string font, float x, float y, vec3 color, std::string text) -> void;
void RenderText(std::string font, float x, float y, vec3 color, bool center, std::string text);
void RenderText(std::string font, float x, float y, float scale, std::string text);
void RenderText(std::string font, float x, float y, vec3 color, float scale, std::string text);
void RenderText(std::string font, float x, float y, vec3 color, float scale, bool center, std::string text);
gl_charmap LoadTextTextures(std::string font, int size);


extern std::map<std::string, gl_charmap> FontCatalogue;