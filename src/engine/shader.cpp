#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <map>
#include <engine/core/types.h>
#include <engine/rvn.h>
#include <engine/serialization/parsing/parser.h>
#include <engine/logging.h>
#include <engine/shader.h>

std::map<std::string, Shader*> ShaderCatalogue;

void Shader::Use()
{
	glUseProgram(this->gl_program_id);
}

void Shader::SetBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(this->gl_program_id, name.c_str()), static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(this->gl_program_id, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(this->gl_program_id, name.c_str()), value);
}

void Shader::SetFloat2(const std::string& name, float value0, float value1) const
{
	glUniform2f(glGetUniformLocation(this->gl_program_id, name.c_str()), value0, value1);
}

void Shader::SetFloat2(const std::string& name, vec2 vec) const
{
	glUniform2f(glGetUniformLocation(this->gl_program_id, name.c_str()), vec.x, vec.y);
}

void Shader::SetFloat3(const std::string& name, float value0, float value1, float value2) const
{
	glUniform3f(glGetUniformLocation(this->gl_program_id, name.c_str()), value0, value1, value2);
}

void Shader::SetFloat3(const std::string& name, vec3 vec) const
{
	glUniform3f(glGetUniformLocation(this->gl_program_id, name.c_str()), vec.x, vec.y, vec.z);
}

void Shader::SetFloat4(const std::string& name, float value0, float value1, float value2, float value3) const
{
	glUniform4f(glGetUniformLocation(this->gl_program_id, name.c_str()), value0, value1, value2, value3);
}

void Shader::SetFloat4(const std::string& name, glm::vec4 vec) const
{
	glUniform4f(glGetUniformLocation(this->gl_program_id, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}

void Shader::SetMatrix4(const std::string& name, glm::mat4 mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(this->gl_program_id, name.c_str()), 1, GL_FALSE, value_ptr(mat));
}



bool check_shader_compile_errors(Shader* shader, std::string type, unsigned int id)
{
	// returns true if we have a compilation problem in the shader program
	int success;
	char infoLog[1024];

	if(type != "PROGRAM")
	{
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(id, 1024, nullptr, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR: " << type << " SHADER AT PROGRAM " << shader->name
			<< "' : vertex shader -> '" << shader->vertex_path << "' fragment shader -> " << shader->fragment_path
			<< "\n DETAILS: \n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			return true;
		}
	}
	else
	{
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if(!success)
		{
			glGetProgramInfoLog(id, 1024, nullptr, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR: AT PROGRAM '" << shader->name
			<< "' : vertex shader -> '" << shader->vertex_path << "' fragment shader -> " << shader->fragment_path
			<< "\n DETAILS: \n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			return true;
		}
	}

	return false;
}

Shader* create_shader_program(
	std::string name,
	const std::string vertex_shader_filename,
	const std::string geometry_shader_filename,
	const std::string fragment_shader_filename)
{
	auto shader = new Shader();
	shader->name = name;

	// OPTIONAL SHADERS
	bool build_geometry_shader = geometry_shader_filename != "";
	u32 optional_shaders[5];
	int optional_shaders_count = 0;

	bool problem = false;

	// > LOAD SHADERS 
	// >> VERTEX
	std::string vertexCode;
	{
		shader->vertex_path = Paths::Shaders + vertex_shader_filename + Paths::ShaderFileExtension;
		std::ifstream vShaderFile;

		vShaderFile.open(shader->vertex_path);
		std::stringstream vShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		vShaderFile.close();
		vertexCode = vShaderStream.str();

		if(vShaderFile.fail())
			std::cout << "ERROR::VERTEX SHADER::FILE_NOT_SUCCESFULLY_READ : " << vertex_shader_filename << std::endl;
	}

	// >> FRAGMENT
	std::string fragmentCode;
	{
		shader->fragment_path = Paths::Shaders + fragment_shader_filename + Paths::ShaderFileExtension;
		std::ifstream fShaderFile;
		fShaderFile.open(shader->fragment_path);
		std::stringstream fShaderStream;
		fShaderStream << fShaderFile.rdbuf();
		fShaderFile.close();
		fragmentCode = fShaderStream.str();

		if(fShaderFile.fail())
			std::cout << "ERROR::FRAGMENT SHADER::FILE_NOT_SUCCESFULLY_READ : " << fragment_shader_filename << std::endl;
	}

	// > COMPILE SHADERS
	// >> VERTEX
	unsigned int vertex;
	{
		const char* vShaderCode = vertexCode.c_str();

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, nullptr);
		glCompileShader(vertex);
		problem = problem || check_shader_compile_errors(shader, "VERTEX", vertex);
	}

	// >> FRAGMENT
	unsigned int fragment;
	{
		const char* fShaderCode = fragmentCode.c_str();

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, nullptr);
		glCompileShader(fragment);
		problem = problem || check_shader_compile_errors(shader, "FRAGMENT", fragment);
	}

	// > ATTACH SHADERS TO PROGRAM
	shader->gl_program_id = glCreateProgram();
	glAttachShader(shader->gl_program_id, vertex);
	glAttachShader(shader->gl_program_id, fragment);


	// > LOAD, COMPILE AND ATTACH OPTIONAL SHADERS 
	// >> GEOMETRY
	if(build_geometry_shader)
	{
		// >>> LOAD
		std::string geometryCode;
		{
			shader->geometry_path = Paths::Shaders + geometry_shader_filename + Paths::ShaderFileExtension;
			std::ifstream gShaderFile;

			gShaderFile.open(shader->geometry_path);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();

			if(gShaderFile.fail())
				std::cout << "ERROR::GEOMETRY SHADER::FILE_NOT_SUCCESFULLY_READ : " << geometry_shader_filename << std::endl;
		}

		// >>> COMPILE
		unsigned int geometry;
		{
			const char* gShaderCode = geometryCode.c_str();

			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, nullptr);
			glCompileShader(geometry);
			problem = problem || check_shader_compile_errors(shader, "GEOMETRY", geometry);
		}

		// >>> ATTACH
		glAttachShader(shader->gl_program_id, geometry);
		optional_shaders[0] = geometry;
		optional_shaders_count++;
	}


	// > LINK PROGRAM
	glLinkProgram(shader->gl_program_id);
	problem = problem || check_shader_compile_errors(shader, "PROGRAM", shader->gl_program_id);


	// > DELETE SHADERS
	// ... as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	for(int i = 0; i < optional_shaders_count; i++)
		glDeleteShader(optional_shaders[i]);


	// > ASSERT AND RETURN
	//if(problem)
	// assert(false);

	return shader;
}

Shader* create_shader_program(std::string name, const std::string vertex_shader_filename, const std::string fragment_shader_filename)
{
	return create_shader_program(name, vertex_shader_filename, "", fragment_shader_filename);
}

void initialize_shaders()
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
