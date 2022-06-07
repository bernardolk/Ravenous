#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <ft2build.h>
#include FT_FREETYPE_H

// prototypes
using strVector = std::vector<std::string>;

Mesh*             load_wavefront_obj_as_mesh          (std::string path, std::string name, bool setup_gl_data, GLenum render_method);
unsigned int      load_texture_from_file              (std::string path, const std::string & directory, bool gamma = false);
gl_charmap        load_text_textures                  (std::string font, int size);
void              attach_extra_data_to_mesh           (std::string filename, std::string filepath, Mesh* mesh);
void              load_mesh_extra_data                (std::string filename, Mesh* mesh);
void              write_mesh_extra_data_file          (std::string filename, Mesh* mesh);
void              load_textures_from_assets_folder    ();
strVector         get_files_in_folder                 (std::string directory);



gl_charmap load_text_textures(std::string font, int size) 
{
	// Load font
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		Quit_fatal("Freetype: Could not init FreeType Library");
      
	FT_Face face;
	std::string filepath = FONTS_PATH + font;
	if (FT_New_Face(ft, filepath.c_str(), 0, &face))
		log(LOG_ERROR, "Freetype: Failed to load font");

	FT_Set_Pixel_Sizes(face, 0, size);

   gl_charmap font_charmap;
	//we will store all characters inside the Characters map
	for (GLubyte c = 0; c < 128; c++)
	{
		//Load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) 
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
         gylphTexture, 
         glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), 
         glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), 
         face->glyph->advance.x 
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
            std::cout << "Texture '" << texture_filename << "' could not be loaded. \n"; 
            assert(false);
         }

         auto ind = texture_filename.find('.');
         std::string texture_name = texture_filename.substr(0, ind);

         std::string texture_type = "texture_diffuse";
         if(texture_name.find("normal") != std::string::npos)
         {
            texture_type = "texture_normal";
         }

         Texture new_texture {
            texture_id,
            texture_type,
            texture_filename,
            texture_name
         };

         Texture_Catalogue.insert({texture_name, new_texture});
      }
   }
}


Mesh* load_wavefront_obj_as_mesh(
   std::string path, std::string filename, std::string name = "", bool setup_gl_data = true, GLenum render_method = GL_TRIANGLES
) {
   /* Loads a model from the provided path and filename and add it to the Geometry_Catalogue with provided name */

   // @todo: Currently, we are loading each vertex using the .obj "f line", but that creates 3 vertex per faces.
   //    For faces that share an exact vertex (like in the case of a box where each face is a quad, sharing one
   //    triangle). This means we must use glDrawArrays instead of glDrawElements. One option is to code a custom
   //    exporter in blender to get just unique vertex data + indices, but maybe for now this is enough.

   std::string full_path = path + filename + ".obj";
	std::ifstream reader(full_path);

   if(!reader.is_open())
   {
      std::cout << "Fatal: Could not find/open wavefront file at '" << full_path << "'.\n";
      assert(false);
   }

	std::string line;
	auto mesh = new Mesh();

   std::vector<vec3> v_pos;
   std::vector<vec2> v_texels;
   std::vector<vec3> v_normals;
   int faces_count = 0;

   // Parses file
	while (getline(reader, line)) 
   {
		const char* cline = line.c_str();
		size_t size = line.size();

		Parser::Parse p{ cline, size };
		p = parse_token(p);
      std::string attr = p.string_buffer;

      if(!p.hasToken)
         continue;

      // ?
		if (attr == "m")
      {

		}

      // vertex coordinates
		else if (attr == "v")
      {
         p = parse_vec3(p);
         v_pos.push_back(p.get_vec3_val());
		}

      // texture coordinates
      else if(attr == "vt")
      {
         p = parse_vec2(p);
         v_texels.push_back(p.get_vec2_val());
      }

      else if(attr == "vn")
      {
         p = parse_vec3(p);
         v_normals.push_back(p.get_vec3_val());
      }

      // construct faces
      else if (attr == "f")
      {
         /* We are dealing now with the format: 'f 1/1/1 2/2/1 3/3/1 4/4/1' which follows the template 'vi/vt/vn' for each vertex.
            We can then use the anticlock-wise winding order to use the 123 341 index convention to get triangles from each f line. 
            If faces are triangulated, then we will have only 3 vertices per face instead of 4 like in the example above.
            Note about indices (mesh->indices): They refer not to the index 'vi' but to the index of the vertex in the mesh->vertices array.
         */

         int vertex_count = 0;
         
         // we expect either faces with 3 or 4 vertices only.
         while(true)
         {
            Vertex v;
            p = parse_all_whitespace(p);

            // parses vertex index
            {
               p = parse_uint(p);
               if(!p.hasToken) break;

               u32 index = p.uiToken - 1;
               v.position = v_pos[index];
            }

            p = parse_symbol(p);
            if(p.hasToken || p.cToken == '/')
            {
               // parses texel index
               p = parse_uint(p);
               if(p.hasToken && v_texels.size() > 0)
               {
                  u32 index = p.uiToken - 1;
                  v.tex_coords = v_texels[index];
               }

               // parses normal index
               p = parse_symbol(p);
               if(p.hasToken || p.cToken == '/')
               {
                  p = parse_uint(p);
                  if(p.hasToken && v_normals.size() > 0)
                  {
                     u32 index = p.uiToken - 1;
                     v.normal = v_normals[index];
                  }
               }
            }

            // adds vertex to mesh, finally
            mesh->vertices.push_back(v);

            vertex_count++;
         }
         
         // face's triangle #1
         int face_index = mesh->vertices.size() - vertex_count;
         mesh->indices.push_back(face_index + 0);
         mesh->indices.push_back(face_index + 1);
         mesh->indices.push_back(face_index + 2);

         if(vertex_count == 4)
         { 
            // face's triangle #2
            mesh->indices.push_back(face_index + 2);
            mesh->indices.push_back(face_index + 3);
            mesh->indices.push_back(face_index + 0);
         }

         else if(vertex_count > 4)
         {
            Quit_fatal("mesh file " + filename + 
               ".obj contain at least one face with unsupported ammount of vertices. Please triangulate or quadfy faces.\n");
         }

         else if(vertex_count < 3)
         {
            Quit_fatal("mesh file " + filename + ".obj contain at least one face with 2 or less vertices. Please review the geometry.\n");
         }

         faces_count++;
      }
	}

   mesh->faces_count = faces_count;

   // sets texture name
   std::string       catalogue_name;
   if(name != "")    catalogue_name = name;
   else              catalogue_name = filename;
   mesh->name = catalogue_name;

   // load/computes tangents and bitangents
   if(v_texels.size() > 0)
      attach_extra_data_to_mesh(filename, path, mesh);

   // setup gl data
   if(setup_gl_data)
      mesh->setup_gl_data();

   // sets render method for mesh
   mesh->render_method = render_method;


   // adds to catalogue
   Geometry_Catalogue.insert({catalogue_name, mesh});

	return mesh;
}


unsigned int load_texture_from_file(std::string filename, const std::string & directory, bool gamma)
{
   // returns the gl_texture ID
   
   std::string path;
   if (path.substr(0, path.length() - 2) == "/")
      path = directory + filename;
   else
      path = directory + "/" + filename;

   int width, height, nrComponents;
   unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
   if (!data)
   {
      std::cout << "Texture failed to load at path: " << path << std::endl;
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


strVector get_files_in_folder(std::string directory)
{
   strVector filenames;
   std::string path_to_files = directory + "\\*";
   WIN32_FIND_DATA files;
   HANDLE find_files_handle = FindFirstFile(path_to_files.c_str(), &files);

   if(find_files_handle == INVALID_HANDLE_VALUE) 
   {
      std::cout << "Error: Invalid directory '" + directory + "' for finding files.";
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


void write_mesh_extra_data_file(std::string filename, Mesh* mesh)
{
   std::string extra_data_path  = MODELS_PATH + "extra_data/" + filename + ".objplus";
   std::ofstream writer(extra_data_path);

   if(!writer.is_open())
      Quit_fatal("couldn't write mesh extra data.");

   writer << std::fixed << std::setprecision(4);

   // write tangents
   For(mesh->vertices.size())
   {
      writer << "vtan"
         << " " << mesh->vertices[i].tangent.x
         << " " << mesh->vertices[i].tangent.y
         << " " << mesh->vertices[i].tangent.z << "\n";
   }

   // write bitangents
   For(mesh->vertices.size())
   {
      writer << "vbitan"
         << " " << mesh->vertices[i].bitangent.x
         << " " << mesh->vertices[i].bitangent.y
         << " " << mesh->vertices[i].bitangent.z << "\n";
   }

   writer.close();

   log(LOG_INFO, "Wrote mesh extra data for " + filename + " mesh.");
}


void load_mesh_extra_data(std::string filename, Mesh* mesh)
{
   std::string extra_data_path  = MODELS_PATH + "extra_data/" + filename + ".objplus";
   
   std::ifstream reader(extra_data_path);
   std::string line;

   u32 vtan_i = 0;
   u32 vbitan_i = 0;
   while(getline(reader, line))
   {
      const char* cline = line.c_str();
		size_t size = line.size();

		Parser::Parse p{ cline, size };
      p = parse_token(p);
      
      if(!p.hasToken)
         continue;
      
      std::string attr = p.string_buffer;

      if(attr == "vtan")
      {
         p = parse_vec3(p);
         mesh->vertices[vtan_i++].tangent = p.get_vec3_val();
      }

      else if(attr == "vbitan")
      {
         p = parse_vec3(p);
         mesh->vertices[vbitan_i++].bitangent = p.get_vec3_val();   
      }
   }

   reader.close();
}


void attach_extra_data_to_mesh(std::string filename, std::string filepath, Mesh* mesh)
{
   /* Attach tangents and bitangents data to the mesh from a precomputation based on mesh vertices.
      If the extra mesh data file is outdated from mesh file or inexistent, compute data and write to it.
      If exists and is up to date, loads extra data from it.
   */

   std::string extra_data_path  = MODELS_PATH + "extra_data/" + filename + ".objplus";
   std::string mesh_path        = filepath + filename + ".obj";

   bool compute_extra_data = false;

   //@todo: platform dependency
   WIN32_FIND_DATA find_data_extra_data;
   HANDLE find_handle = FindFirstFileA(extra_data_path.c_str(), &find_data_extra_data);
   if(find_handle != INVALID_HANDLE_VALUE)
   {
      WIN32_FIND_DATA find_data_mesh;
      HANDLE find_handle_mesh = FindFirstFileA(mesh_path.c_str(), &find_data_mesh);
      if(find_handle_mesh == INVALID_HANDLE_VALUE)
         Quit_fatal("Unexpected: couldn't find file handle for mesh obj while checking for extra mesh data.");

      if(CompareFileTime(&find_data_mesh.ftLastWriteTime, &find_data_extra_data.ftLastWriteTime) == 1)
         compute_extra_data = true;

      FindClose(find_handle_mesh);
   }
   else
      compute_extra_data = true;


   if(compute_extra_data)
   {
      mesh->compute_tangents_and_bitangents();
      write_mesh_extra_data_file(filename, mesh);
   }
   else
   {
      load_mesh_extra_data(filename, mesh);
   }

   FindClose(find_handle);
}