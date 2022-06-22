#pragma once

void render_text(float x, float y, std::string text);
void render_text(float x, float y, vec3 color, std::string text);
void render_text(std::string font, float x, float y, std::string text);
void render_text(std::string font, float x, float y, bool center, std::string text);
void render_text(std::string font, float x, float y, vec3 color, std::string text);
void render_text(std::string font, float x, float y, vec3 color, bool center, std::string text);
void render_text(std::string font, float x, float y, float scale,std::string text);
void render_text(std::string font, float x, float y, vec3 color, float scale, std::string text);
void render_text(std::string font, float x, float y, vec3 color, float scale, bool center, std::string text);
gl_charmap load_text_textures(std::string font, int size);

extern std::map<std::string, gl_charmap>   Font_Catalogue;