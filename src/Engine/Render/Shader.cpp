#include "Shader.h"
#ifndef GLAD_INCL
#define GLAD_INCL
#include <glad/glad.h>
#endif
#include <sstream>
#include <engine/core/types.h>
#include <engine/rvn.h>
#include <engine/serialization/parsing/parser.h>

map<string, RShader*> ShaderCatalogue;

void RShader::Use()
{
	glUseProgram(this->gl_program_id);
}

void RShader::SetBool(const string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(this->gl_program_id, name.c_str()), static_cast<int>(value));
}

void RShader::SetInt(const string& name, int value) const
{
	glUniform1i(glGetUniformLocation(this->gl_program_id, name.c_str()), value);
}

void RShader::SetFloat(const string& name, float value) const
{
	glUniform1f(glGetUniformLocation(this->gl_program_id, name.c_str()), value);
}

void RShader::SetFloat2(const string& name, float value0, float value1) const
{
	glUniform2f(glGetUniformLocation(this->gl_program_id, name.c_str()), value0, value1);
}

void RShader::SetFloat2(const string& name, vec2 vec) const
{
	glUniform2f(glGetUniformLocation(this->gl_program_id, name.c_str()), vec.x, vec.y);
}

void RShader::SetFloat3(const string& name, float value0, float value1, float value2) const
{
	glUniform3f(glGetUniformLocation(this->gl_program_id, name.c_str()), value0, value1, value2);
}

void RShader::SetFloat3(const string& name, vec3 vec) const
{
	glUniform3f(glGetUniformLocation(this->gl_program_id, name.c_str()), vec.x, vec.y, vec.z);
}

void RShader::SetFloat4(const string& name, float value0, float value1, float value2, float value3) const
{
	glUniform4f(glGetUniformLocation(this->gl_program_id, name.c_str()), value0, value1, value2, value3);
}

void RShader::SetFloat4(const string& name, glm::vec4 vec) const
{
	glUniform4f(glGetUniformLocation(this->gl_program_id, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}

void RShader::SetMatrix4(const string& name, glm::mat4 mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(this->gl_program_id, name.c_str()), 1, GL_FALSE, value_ptr(mat));
}



bool CheckShaderCompileErrors(RShader* shader, string type, unsigned int id)
{
	// returns true if we have a compilation problem in the shader program
	int success;
	char infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(id, 1024, nullptr, infoLog);
			print("ERROR::SHADER_COMPILATION_ERROR: '%s' SHADER AT PROGRAM '%s': vertex shader -> '%s' fragment shader -> %s\n DETAILS: \n %s  \n-- --------------------------------------------------- -- ", type.c_str(), shader->name.c_str(), shader->vertex_path.c_str(), shader->fragment_path.c_str(), infoLog)
			return true;
		}
	}
	else
	{
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(id, 1024, nullptr, infoLog);
			print("ERROR::PROGRAM_LINKING_ERROR: AT PROGRAM '%s': vertex shader -> '%s' fragment shader -> %s\n DETAILS: \n %s  \n-- --------------------------------------------------- -- ", shader->name.c_str(), shader->vertex_path.c_str(), shader->fragment_path.c_str(), infoLog)
			return true;
		}
	}

	return false;
}

RShader* CreateShaderProgram(string name, const string vertex_shader_filename, const string geometry_shader_filename, const string fragment_shader_filename)
{
	auto shader = new RShader();
	shader->name = name;

	// OPTIONAL SHADERS
	bool build_geometry_shader = geometry_shader_filename != "";
	uint optional_shaders[5];
	int optional_shaders_count = 0;

	bool problem = false;

	// > LOAD SHADERS 
	// >> VERTEX
	string vertexCode;
	{
		shader->vertex_path = Paths::Shaders + vertex_shader_filename + Paths::ShaderFileExtension;
		std::ifstream vShaderFile;

		vShaderFile.open(shader->vertex_path);
		std::stringstream vShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		vShaderFile.close();
		vertexCode = vShaderStream.str();

		if (vShaderFile.fail())
			print("ERROR::VERTEX SHADER::FILE_NOT_SUCCESFULLY_READ : %s", vertex_shader_filename.c_str());
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

		if (fShaderFile.fail())
			print("ERROR::FRAGMENT SHADER::FILE_NOT_SUCCESFULLY_READ : %s", fragment_shader_filename.c_str());
	}

	// > COMPILE SHADERS
	// >> VERTEX
	unsigned int vertex;
	{
		const char* vShaderCode = vertexCode.c_str();

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, nullptr);
		glCompileShader(vertex);
		problem = problem || CheckShaderCompileErrors(shader, "VERTEX", vertex);
	}

	// >> FRAGMENT
	unsigned int fragment;
	{
		const char* fShaderCode = fragmentCode.c_str();

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, nullptr);
		glCompileShader(fragment);
		problem = problem || CheckShaderCompileErrors(shader, "FRAGMENT", fragment);
	}

	// > ATTACH SHADERS TO PROGRAM
	shader->gl_program_id = glCreateProgram();
	glAttachShader(shader->gl_program_id, vertex);
	glAttachShader(shader->gl_program_id, fragment);


	// > LOAD, COMPILE AND ATTACH OPTIONAL SHADERS 
	// >> GEOMETRY
	if (build_geometry_shader)
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

			if (gShaderFile.fail())
				print("ERROR::GEOMETRY SHADER::FILE_NOT_SUCCESFULLY_READ : %s", geometry_shader_filename.c_str());
		}

		// >>> COMPILE
		unsigned int geometry;
		{
			const char* gShaderCode = geometryCode.c_str();

			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, nullptr);
			glCompileShader(geometry);
			problem = problem || CheckShaderCompileErrors(shader, "GEOMETRY", geometry);
		}

		// >>> ATTACH
		glAttachShader(shader->gl_program_id, geometry);
		optional_shaders[0] = geometry;
		optional_shaders_count++;
	}


	// > LINK PROGRAM
	glLinkProgram(shader->gl_program_id);
	problem = problem || CheckShaderCompileErrors(shader, "PROGRAM", shader->gl_program_id);


	// > DELETE SHADERS
	// ... as they're linked into our program now and no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	for (int i = 0; i < optional_shaders_count; i++)
		glDeleteShader(optional_shaders[i]);


	// > ASSERT AND RETURN
	//if(problem)
	// assert(false);

	return shader;
}

RShader* CreateShaderProgram(std::string name, const std::string vertex_shader_filename, const std::string fragment_shader_filename)
{
	return CreateShaderProgram(name, vertex_shader_filename, "", fragment_shader_filename);
}
