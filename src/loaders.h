#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

Mesh* load_wavefront_obj_as_mesh(string path, string name, bool setup_gl_data, GLenum render_method);
unsigned int load_texture_from_file(string path, const string& directory, bool gamma = false);
vector<string> get_files_in_folder(string directory);
void load_textures_from_assets_folder();
gl_charmap load_text_textures(string font, int size);

gl_charmap load_text_textures(string font, int size) 
{
	// Load font
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
      
	FT_Face face;
	string filepath = FONTS_PATH + font;
	if (FT_New_Face(ft, filepath.c_str(), 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
   else
		std::cout << "OK::FREETYPE: " << font << " loaded." << std::endl;

	FT_Set_Pixel_Sizes(face, 0, size);

   gl_charmap font_charmap;
	//we will store all characters inside the Characters map
	for (GLubyte c = 0; c < 128; c++)
	{
		//Load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
			continue;
		}

		GLuint gylphTexture;
		glGenTextures(1, &gylphTexture);
		glBindTexture(GL_TEXTURE_2D, gylphTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                                                                                                         face->glyph->bitmap.buffer);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = { 
         gylphTexture, 
         glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), 
         glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), 
         face->glyph->advance.x 
      };
		font_charmap.insert(std::pair<GLchar, Character>(c, character));
		//std::cout << "c: " << (GLchar)c << " sizeInfo: " << character.Size.x << " (x) " << character.Size.y << " (y)" << std::endl;
	}

   // saves font chars to catalogue
   auto separator = font.find('.');
   string font_name = font.substr(0, separator);
   string font_catalogue_name = font_name + to_string(size);

   Font_Catalogue.insert({font_catalogue_name, font_charmap});

	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

   return font_charmap;
}

void load_textures_from_assets_folder()
{
   auto filenames = get_files_in_folder(TEXTURES_PATH);
   if(filenames.size() > 0)
   {
      for(auto const& texture_filename: filenames)
      {
         unsigned int texture_id = load_texture_from_file(texture_filename, TEXTURES_PATH);

         if(texture_id == 0)
         {
            cout << "Texture '" << texture_filename << "' could not be loaded. \n"; 
            assert(false);
         }

         auto ind = texture_filename.find('.');
         string texture_name = texture_filename.substr(0, ind);

         Texture new_texture {
            texture_id,
            "texture_diffuse",
            texture_filename,
            texture_name
         };

         Texture_Catalogue.insert({texture_name, new_texture});
      }
   }
}


Mesh* load_wavefront_obj_as_mesh(string path, string name, bool setup_gl_data = true, GLenum render_method = GL_TRIANGLES) {
   // this will insert to catalogue!

   string full_path = path + name + ".obj";
	ifstream reader(full_path);

   if(!reader.is_open())
   {
      cout << "Fatal: Could not find/open wavefront file at '" << full_path << "'.\n";
      assert(false);
   }

	std::string line;
	auto mesh = new Mesh();

   // Parses file
	while (getline(reader, line)) {
		const char* cline = line.c_str();
		size_t size = line.size();

		Parser::Parse p{ cline, size };
		p = parse_letter(p);
		if (p.hasToken && p.cToken == 'm') {

		}
		if (p.hasToken && p.cToken == 'v') {
		Vertex vert;
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_float(p);
		  vert.position.x = p.fToken;
		  
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_float(p);
		  vert.position.y = p.fToken;

		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_float(p);
		  vert.position.z = p.fToken;
		
		  mesh->vertices.push_back(vert);
		}

		if (p.hasToken && p.cToken == 'f') {
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_uint(p);
		  mesh->indices.push_back(p.uiToken - 1);
		  
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_uint(p);
		  mesh->indices.push_back(p.uiToken - 1);
		  
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_uint(p);
		  mesh->indices.push_back(p.uiToken - 1);
		}
	}

   // setup other data
   mesh->name = name;
   if(setup_gl_data)
      mesh->setup_gl_data();
   mesh->render_method = render_method;

   // adds to catalogue
   Geometry_Catalogue.insert({name, mesh});

	return mesh;
}


unsigned int load_texture_from_file(string filename, const string& directory, bool gamma)
{
   // returns the gl_texture ID
   
   string path;
   if (path.substr(0, path.length() - 2) == "/")
      path = directory + filename;
   else
      path = directory + "/" + filename;

   int width, height, nrComponents;
   unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
   if (!data)
   {
      cout << "Texture failed to load at path: " << path << endl;
      stbi_image_free(data);
      return 0;
   }
    
   // sets color channel format
   GLenum format;
   switch(nrComponents)
   {
      case 1:
         format = GL_RED;
         break;
      case 3:
         format = GL_RGB;
         break;
      case 4:
         format = GL_RGBA;
         break;
   }

   unsigned int textureID;
   glGenTextures(1, &textureID);
   glBindTexture(GL_TEXTURE_2D, textureID);
   glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
   glGenerateMipmap(GL_TEXTURE_2D);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   stbi_image_free(data);
   return textureID;
}


vector<string> get_files_in_folder(string directory)
{
   vector<string> filenames;
   string path_to_files = directory + "\\*";
   WIN32_FIND_DATA files;
   HANDLE find_files_handle = FindFirstFile(path_to_files.c_str(), &files);

   if(find_files_handle == INVALID_HANDLE_VALUE) 
   {
      cout << "Error: Invalid directory '" + directory + "' for finding files.";
      return filenames;
   }
   
   do {
      int a = strcmp(files.cFileName, ".");
      int b = strcmp(files.cFileName, "..");
      if(!(a == 0 || b == 0))
         filenames.push_back(files.cFileName);
   } while(FindNextFile(find_files_handle, &files));

   FindClose(find_files_handle);
   
   return filenames;
}
