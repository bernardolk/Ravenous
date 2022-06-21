#include <map>
#include <string>
#include <engine/core/rvn_types.h>
#include <texture.h>
#include <engine/catalogues.h>

std::map<std::string, Mesh*>        Geometry_Catalogue;
std::map<std::string, Shader*>      Shader_Catalogue;
std::map<std::string, Texture>      Texture_Catalogue;
std::map<std::string, gl_charmap>   Font_Catalogue;