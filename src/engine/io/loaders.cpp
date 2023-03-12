// @TODO: abstract platform layer away
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include "engine/geometry/vertex.h"
#include "engine/core/logging.h"
#include <glm/gtx/quaternion.hpp>
#include "engine/geometry/mesh.h"
#include <engine/collision/collision_mesh.h>
#include <glad/glad.h>
#include <engine/serialization/parsing/parser.h>
#include "engine/io/loaders.h"
#include "engine/rvn.h"
#include "engine/io/display.h"
#include "engine/render/shader.h"

void load_textures_from_assets_folder()
{
	auto filenames = get_files_in_folder(Paths::Textures);
	if(filenames.size() > 0)
	{
		for(const auto& texture_filename : filenames)
		{
			unsigned int texture_id = load_texture_from_file(texture_filename, Paths::Textures);

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

			Texture new_texture{
			texture_id,
			texture_type,
			texture_filename,
			texture_name
			};

			TextureCatalogue.insert({texture_name, new_texture});
		}
	}
}

Mesh* load_wavefront_obj_as_mesh(
	const std::string& path,
	const std::string& filename,
	const std::string& name,
	bool setup_gl_data,
	RenderMethodEnum render_method)
{
	/* Loads a model from the provided path and filename and add it to the Geometry_Catalogue with provided name */

	// @todo: Currently, we are loading each vertex using the .obj "f line", but that creates 3 vertex per faces.
	//    For faces that share an exact vertex (like in the case of a box where each face is a quad, sharing one
	//    triangle). This means we must use glDrawArrays instead of glDrawElements. One option is to code a custom
	//    exporter in blender to get just unique vertex data + indices, but maybe for now this is enough.

	const auto full_path = path + filename + ".obj";
	Parser p{full_path};

	// @TODO: use a memory pool
	auto mesh = new Mesh();

	std::vector<vec3> v_pos;
	std::vector<vec2> v_texels;
	std::vector<vec3> v_normals;
	int faces_count = 0;

	// Parses file
	while(p.NextLine())
	{
		p.ParseToken();
		const auto attr = get_parsed<std::string>(p);

		if(!p.HasToken())
			continue;

		if(attr == "m")
		{
			// ?
		}

		// vertex coordinates
		else if(attr == "v")
		{
			p.ParseVec3();
			v_pos.push_back(get_parsed<glm::vec3>(p));
		}

		// texture coordinates
		else if(attr == "vt")
		{
			p.ParseVec2();
			v_texels.push_back(get_parsed<glm::vec2>(p));
		}

		else if(attr == "vn")
		{
			p.ParseVec3();
			v_normals.push_back(get_parsed<glm::vec3>(p));
		}

		// construct faces
		else if(attr == "f")
		{
			/* We are dealing now with the format: 'f 1/1/1 2/2/1 3/3/1 4/4/1' which follows the template 'vi/vt/vn' for each vertex.
			   We can then use the anticlockwise winding order to use the 123 341 index convention to get triangles from each f line. 
			   If faces are triangulated, then we will have only 3 vertices per face instead of 4 like in the example above.
			   Note about indices (mesh->indices): They refer not to the index 'vi' but to the index of the vertex in the mesh->vertices array.
			*/

			int number_of_vertexes_in_face = 0;

			// iterate over face's vertices. We expect either faces with 3 or 4 vertices only.
			while(true)
			{
				Vertex v;
				p.ParseAllWhitespace();

				// parses vertex index
				{
					p.ParseUint();
					if(!p.HasToken())
						break;

					u32 index = get_parsed<u32>(p) - 1;
					v.position = v_pos[index];
				}

				p.ParseSymbol();
				if(p.HasToken() || get_parsed<char>(p) == '/')
				{
					// parses texel index
					p.ParseUint();
					if(p.HasToken() && !v_texels.empty())
					{
						u32 index = get_parsed<u32>(p) - 1;
						v.tex_coords = v_texels[index];
					}

					// parses normal index
					p.ParseSymbol();
					if(p.HasToken() || get_parsed<char>(p) == '/')
					{
						p.ParseUint();
						if(p.HasToken() && !v_normals.empty())
						{
							u32 index = get_parsed<u32>(p) - 1;
							v.normal = v_normals[index];
						}
					}
				}

				// adds vertex to mesh, finally
				mesh->vertices.push_back(v);

				number_of_vertexes_in_face++;
			}


			// Creates the faces respecting winding order: Assumes that each face has unique vertices.
			// Index vector reads like: T0_v0, T0_v1, T0_v2, T1_v0, T1_v1, T1_v2, T2_v0 ... where T is triangle and v is vertex.
			// face's triangle #1
			int first_vertex_index_in_face = mesh->vertices.size() - number_of_vertexes_in_face;
			mesh->indices.push_back(first_vertex_index_in_face + 0);
			mesh->indices.push_back(first_vertex_index_in_face + 1);
			mesh->indices.push_back(first_vertex_index_in_face + 2);

			if(number_of_vertexes_in_face == 4)
			{
				// face's triangle #2
				mesh->indices.push_back(first_vertex_index_in_face + 2);
				mesh->indices.push_back(first_vertex_index_in_face + 3);
				mesh->indices.push_back(first_vertex_index_in_face + 0);

				faces_count += 2;
			}

			else if(number_of_vertexes_in_face > 4)
			{
				Quit_fatal("mesh file " + filename +
					".obj contain at least one face with unsupported ammount of vertices. Please triangulate or quadfy faces.\n");
			}

			else if(number_of_vertexes_in_face < 3)
			{
				Quit_fatal("mesh file " + filename + ".obj contain at least one face with 2 or less vertices. Please review the geometry.\n");
			}
			else
			{
				faces_count++;
			}

		}
	}

	mesh->faces_count = faces_count;

	// load/computes tangents and bitangents
	if(!v_texels.empty())
		attach_extra_data_to_mesh(filename, path, mesh);

	// setup gl data
	if(setup_gl_data)
		mesh->SetupGLData();

	// sets render method for mesh
	mesh->render_method = static_cast<u32>(render_method);


	// sets texture name and adds to catalogue
	std::string catalogue_name = !name.empty() ? name : filename;
	mesh->name = catalogue_name;
	GeometryCatalogue.insert({catalogue_name, mesh});

	return mesh;
}

CollisionMesh* load_wavefront_obj_as_collision_mesh(std::string path, std::string filename, std::string name)
{
	/* Loads a model from the provided path and filename and add it to the Collision_Geometry_Catalogue with provided name */

	const auto full_path = path + filename + ".obj";
	Parser p{full_path};

	// @TODO: Use a memory pool
	auto c_mesh = new CollisionMesh();

	// Parses file
	while(p.NextLine())
	{
		p.ParseToken();
		const auto attr = get_parsed<std::string>(p);

		if(!p.HasToken())
			continue;

		// vertex coordinates
		if(attr == "v")
		{
			p.ParseVec3();
			c_mesh->vertices.push_back(get_parsed<glm::vec3>(p));
		}

		// construct faces
		else if(attr == "f")
		{
			int number_of_vertexes_in_face = 0;
			u32 index_buffer[4];
			// iterate over face's vertices
			while(true)
			{
				p.ParseAllWhitespace();

				// parses vertex index
				{
					p.ParseUint();
					if(!p.HasToken())
						break;

					// corrects from 1-first-element convention (from .obj file) to 0-first.
					const u32 index = get_parsed<u32>(p) - 1;
					// c_mesh.index.push_back(index);
					index_buffer[number_of_vertexes_in_face] = index;
				}

				// discard remaining face's vertex info
				p.ParseSymbol();
				p.ParseUint();
				p.ParseSymbol();
				p.ParseUint();

				number_of_vertexes_in_face++;
			}

			// Creates the faces respecting winding order: Assumes that each face has unique vertices.
			// Index vector reads like: T0_v0, T0_v1, T0_v2, T1_v0, T1_v1, T1_v2, T2_v0 ... where T is triangle and v is vertex position.
			// face's triangle #1
			c_mesh->indices.push_back(index_buffer[0]);
			c_mesh->indices.push_back(index_buffer[1]);
			c_mesh->indices.push_back(index_buffer[2]);

			if(number_of_vertexes_in_face == 4)
			{
				// face's triangle #2
				c_mesh->indices.push_back(index_buffer[2]);
				c_mesh->indices.push_back(index_buffer[3]);
				c_mesh->indices.push_back(index_buffer[0]);
			}
		}
	}

	// adds to catalogue
	const std::string catalogue_name = !name.empty() ? name : filename;
	CollisionGeometryCatalogue.insert({catalogue_name, c_mesh});

	return c_mesh;
}

unsigned int load_texture_from_file(const std::string& filename, const std::string& directory, bool gamma)
{
	// returns the gl_texture ID

	std::string path;
	if(path.substr(0, path.length() - 2) == "/")
		path = directory + filename;
	else
		path = directory + "/" + filename;

	int width, height, nr_components;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nr_components, 0);
	if(!data)
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		return 0;
	}

	// sets color channel format
	GLenum format;
	switch(nr_components)
	{
		case 1: format = GL_RED;
			break;
		case 3: format = GL_RGB;
			break;
		case 4: format = GL_RGBA;
			break;
		default: ;
	}

	unsigned int texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	return texture_id;
}


StrVec get_files_in_folder(std::string directory)
{
	StrVec filenames;
	std::string path_to_files = directory + "\\*";
	WIN32_FIND_DATA files;
	HANDLE find_files_handle = FindFirstFile(path_to_files.c_str(), &files);

	if(find_files_handle == INVALID_HANDLE_VALUE)
	{
		std::cout << "Error: Invalid directory '" + directory + "' for finding files.";
		return filenames;
	}

	do
	{
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
	const auto extra_data_path = Paths::Models + "extra_data/" + filename + ".objplus";
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
	const auto extra_data_path = Paths::Models + "extra_data/" + filename + ".objplus";
	Parser p{extra_data_path};

	u32 vtan_i = 0;
	u32 vbitan_i = 0;
	while(p.NextLine())
	{
		p.ParseToken();

		if(!p.HasToken())
			continue;

		auto attr = get_parsed<std::string>(p);

		if(attr == "vtan")
		{
			p.ParseVec3();
			mesh->vertices[vtan_i++].tangent = get_parsed<glm::vec3>(p);
		}

		else if(attr == "vbitan")
		{
			p.ParseVec3();
			mesh->vertices[vbitan_i++].bitangent = get_parsed<glm::vec3>(p);
		}
	}
}


void attach_extra_data_to_mesh(std::string filename, std::string filepath, Mesh* mesh)
{
	/* Attach tangents and bitangents data to the mesh from a precomputation based on mesh vertices.
	   If the extra mesh data file is outdated from mesh file or inexistent, compute data and write to it.
	   If exists and is up to date, loads extra data from it.
	*/

	std::string extra_data_path = Paths::Models + "extra_data/" + filename + ".objplus";
	std::string mesh_path = filepath + filename + ".obj";

	bool compute_extra_data = false;

	//@todo: platform dependency
	WIN32_FIND_DATA find_data_extra_data;
	HANDLE find_handle = FindFirstFileA(extra_data_path.c_str(), &find_data_extra_data);
	if(find_handle != INVALID_HANDLE_VALUE)
	{
		WIN32_FIND_DATA find_data_mesh;
		HANDLE find_handle_mesh = FindFirstFileA(mesh_path.c_str(), &find_data_mesh);
		if(find_handle_mesh == INVALID_HANDLE_VALUE)
			Quit_fatal("Unexpected: couldn't find file handle for mesh obj while checking for extra mesh data.")

		if(CompareFileTime(&find_data_mesh.ftLastWriteTime, &find_data_extra_data.ftLastWriteTime) == 1)
			compute_extra_data = true;

		FindClose(find_handle_mesh);
	}
	else
		compute_extra_data = true;


	if(compute_extra_data)
	{
		mesh->ComputeTangentsAndBitangents();
		write_mesh_extra_data_file(filename, mesh);
	}
	else
	{
		load_mesh_extra_data(filename, mesh);
	}

	FindClose(find_handle);
}


void LoadShaders()
{
	/* Parses shader program info from programs file, assembles each shader program and stores them into the
	   shaders catalogue. */

	Parser p{Paths::Shaders + "programs.csv"};

	// discard header
	p.NextLine();

	while(p.NextLine())
	{
		bool error = false, missing_comma = false, has_geometry_shader = false;

		p.ParseToken();
		if(!p.HasToken())
			error = true;
		const auto shader_name = get_parsed<std::string>(p);

		p.ParseAllWhitespace();
		p.ParseSymbol();
		if(!p.HasToken())
			missing_comma = true;

		p.ParseAllWhitespace();
		p.ParseToken();
		if(!p.HasToken())
			error = true;
		const auto vertex_shader_name = get_parsed<std::string>(p);

		p.ParseAllWhitespace();
		p.ParseSymbol();
		if(!p.HasToken())
			missing_comma = true;

		p.ParseAllWhitespace();
		p.ParseToken();
		if(p.HasToken())
			has_geometry_shader = true;
		const auto geometry_shader_name = get_parsed<std::string>(p);

		p.ParseAllWhitespace();
		p.ParseSymbol();
		if(!p.HasToken())
			missing_comma = true;

		p.ParseAllWhitespace();
		p.ParseToken();
		if(!p.HasToken())
			error = true;
		const auto fragment_shader_name = get_parsed<std::string>(p);

		// load shaders code and mounts program from parsed shader attributes
		Shader* shader;
		if(has_geometry_shader)
			shader = create_shader_program(shader_name, vertex_shader_name, geometry_shader_name, fragment_shader_name);
		else
			shader = create_shader_program(shader_name, vertex_shader_name, fragment_shader_name);

		ShaderCatalogue.insert({shader->name, shader});

		if(error)
			Quit_fatal("Error in shader programs file definition. Couldn't parse line " + std::to_string(p.line_count) + ".");
		if(missing_comma)
			Quit_fatal("Error in shader programs file definition. There is a missing comma in line " + std::to_string(p.line_count) + ".");
	}

	// setup for text shader
	auto text_shader = ShaderCatalogue.find("text")->second;
	text_shader->Use();
	text_shader->SetMatrix4("projection", glm::ortho(0.0f, GlobalDisplayConfig::viewport_width, 0.0f, GlobalDisplayConfig::viewport_height));
}