#pragma once

struct Mesh;
struct Shader;
struct Texture;

extern std::map<std::string, Mesh*>        Geometry_Catalogue;
extern std::map<std::string, Shader*>      Shader_Catalogue;
extern std::map<std::string, Texture>      Texture_Catalogue;
extern std::map<std::string, gl_charmap>   Font_Catalogue;