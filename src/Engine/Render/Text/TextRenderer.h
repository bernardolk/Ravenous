#pragma once

struct RCharacter;

using gl_charmap = map<char, RCharacter>;

void RenderText(float X, float Y, string Text);
void RenderText(float X, float Y, vec3 Color, string Text);
void RenderText(string Font, float X, float Y, string Text);
void RenderText(string Font, float X, float Y, bool Center, string Text);
auto RenderText(string Font, float X, float Y, vec3 Color, string Text) -> void;
void RenderText(string Font, float X, float Y, vec3 Color, bool Center, string Text);
void RenderText(string Font, float X, float Y, float Scale, string Text);
void RenderText(string Font, float X, float Y, vec3 Color, float Scale, string Text);
void RenderText(string Font, float X, float Y, vec3 Color, float Scale, bool Center, string Text);
gl_charmap LoadTextTextures(string Font, int Size);


extern map<string, gl_charmap> FontCatalogue;
