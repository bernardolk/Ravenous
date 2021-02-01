#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

struct MeshData {
	std::vector<Vertex> vertexes;
	std::vector<unsigned int> indices;
	unsigned int faceCount = 0;
};

unsigned int load_texture_from_file(std::string path, const string& directory, bool gamma = false);
MeshData import_wavefront_obj(std::string path);

MeshData import_wavefront_obj(std::string path) {
	ifstream reader(path);
	std::string line;

	MeshData mdata;

	while (getline(reader, line)) {
		//istringstream iss(line);
		const char* cline = line.c_str();
		size_t size = line.size();

		Parse p{ cline, size };
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
		
		  mdata.vertexes.push_back(vert);
		}

		if (p.hasToken && p.cToken == 'f') {
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_uint(p);
		  mdata.indices.push_back(p.uiToken - 1);
		  
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_uint(p);
		  mdata.indices.push_back(p.uiToken - 1);
		  
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_uint(p);
		  mdata.indices.push_back(p.uiToken - 1);
		}
		mdata.faceCount ++;
	}

	return mdata;
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
