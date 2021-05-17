#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

Mesh* load_wavefront_obj_as_mesh(string path, string name, bool setup_gl_data, GLenum render_method);
unsigned int load_texture_from_file(string path, const string& directory, bool gamma = false);


Mesh* load_wavefront_obj_as_mesh(string path, string name, bool setup_gl_data = true, GLenum render_method = GL_TRIANGLES) {
   // this will insert to catalogue!

	ifstream reader(path);

   if(!reader.is_open())
   {
      cout << "Fatal: Could not open .obj file at '" << path << "'.\n";
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


// returns the gl_texture ID
unsigned int load_texture_from_file(std::string filename, const std::string& directory, bool gamma)
{
    std::string path;
    if (path.substr(0, path.length() - 2) == "/")
        path = directory + filename;
    else
        path = directory + "/" + filename;

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
      unsigned int textureID;
      glGenTextures(1, &textureID);

      GLenum format;
      if (nrComponents == 1)
         format = GL_RED;
      else if (nrComponents == 3)
         format = GL_RGB;
      else if (nrComponents == 4)
         format = GL_RGBA;

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
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);

        return 0;
    }
}
