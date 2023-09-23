#pragma once

struct Character;

using gl_charmap = map<char, Character>;

void RenderText(float x, float y, string text);
void RenderText(float x, float y, vec3 color, string text);
void RenderText(string font, float x, float y, string text);
void RenderText(string font, float x, float y, bool center, string text);
auto RenderText(string font, float x, float y, vec3 color, string text) -> void;
void RenderText(string font, float x, float y, vec3 color, bool center, string text);
void RenderText(string font, float x, float y, float scale, string text);
void RenderText(string font, float x, float y, vec3 color, float scale, string text);
void RenderText(string font, float x, float y, vec3 color, float scale, bool center, string text);
gl_charmap LoadTextTextures(string font, int size);


extern map<string, gl_charmap> FontCatalogue;
