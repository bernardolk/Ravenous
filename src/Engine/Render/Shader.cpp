#include "Shader.h"
#ifndef GLAD_INCL
#define GLAD_INCL
#include <glad/glad.h>
#endif
#include <sstream>
#include <engine/core/Types.h>
#include <engine/rvn.h>
#include <engine/serialization/parsing/parser.h>

map<string, RShader*> ShaderCatalogue;

void RShader::Use()
{
	glUseProgram(this->GLProgramID);
}

void RShader::SetBool(const string& Name, bool Value) const
{
	glUniform1i(glGetUniformLocation(this->GLProgramID, Name.c_str()), static_cast<int>(Value));
}

void RShader::SetInt(const string& Name, int Value) const
{
	glUniform1i(glGetUniformLocation(this->GLProgramID, Name.c_str()), Value);
}

void RShader::SetFloat(const string& Name, float Value) const
{
	glUniform1f(glGetUniformLocation(this->GLProgramID, Name.c_str()), Value);
}

void RShader::SetFloat2(const string& Name, float Value0, float Value1) const
{
	glUniform2f(glGetUniformLocation(this->GLProgramID, Name.c_str()), Value0, Value1);
}

void RShader::SetFloat2(const string& Name, vec2 Vec) const
{
	glUniform2f(glGetUniformLocation(this->GLProgramID, Name.c_str()), Vec.x, Vec.y);
}

void RShader::SetFloat3(const string& Name, float Value0, float Value1, float Value2) const
{
	glUniform3f(glGetUniformLocation(this->GLProgramID, Name.c_str()), Value0, Value1, Value2);
}

void RShader::SetFloat3(const string& Name, vec3 Vec) const
{
	glUniform3f(glGetUniformLocation(this->GLProgramID, Name.c_str()), Vec.x, Vec.y, Vec.z);
}

void RShader::SetFloat4(const string& Name, float Value0, float Value1, float Value2, float Value3) const
{
	glUniform4f(glGetUniformLocation(this->GLProgramID, Name.c_str()), Value0, Value1, Value2, Value3);
}

void RShader::SetFloat4(const string& Name, glm::vec4 Vec) const
{
	glUniform4f(glGetUniformLocation(this->GLProgramID, Name.c_str()), Vec.x, Vec.y, Vec.z, Vec.w);
}

void RShader::SetMatrix4(const string& Name, glm::mat4 Mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(this->GLProgramID, Name.c_str()), 1, GL_FALSE, glm::value_ptr(Mat));
}



bool CheckShaderCompileErrors(RShader* Shader, string Type, unsigned int Id)
{
	// returns true if we have a compilation Problem in the Shader program
	int Success;
	char InfoLog[1024];

	if (Type != "PROGRAM")
	{
		glGetShaderiv(Id, GL_COMPILE_STATUS, &Success);
		if (!Success)
		{
			glGetShaderInfoLog(Id, 1024, nullptr, InfoLog);
			print("ERROR::SHADER_COMPILATION_ERROR: '%s' SHADER AT PROGRAM '%s': Vertex Shader -> '%s' Fragment Shader -> %s\n DETAILS: \n %s  \n-- --------------------------------------------------- -- ", Type.c_str(), Shader->Name.c_str(), Shader->VertexPath.c_str(), Shader->FragmentPath.c_str(), InfoLog)
			return true;
		}
	}
	else
	{
		glGetProgramiv(Id, GL_LINK_STATUS, &Success);
		if (!Success)
		{
			glGetProgramInfoLog(Id, 1024, nullptr, InfoLog);
			print("ERROR::PROGRAM_LINKING_ERROR: AT PROGRAM '%s': Vertex Shader -> '%s' Fragment Shader -> %s\n DETAILS: \n %s  \n-- --------------------------------------------------- -- ", Shader->Name.c_str(), Shader->VertexPath.c_str(), Shader->FragmentPath.c_str(), InfoLog)
			return true;
		}
	}

	return false;
}

RShader* CreateShaderProgram(string Name, const string VertexShaderFilename, const string GeometryShaderFilename, const string FragmentShaderFilename)
{
	auto Shader = new RShader();
	Shader->Name = Name;

	// OPTIONAL SHADERS
	bool BuildGeometryShader = GeometryShaderFilename != "";
	uint OptionalShaders[5];
	int OptionalShadersCount = 0;

	bool Problem = false;

	// > LOAD SHADERS 
	// >> VERTEX
	string VertexCode;
	{
		Shader->VertexPath = Paths::Shaders + VertexShaderFilename + Paths::ShaderFileExtension;
		
		std::ifstream VShaderFile;
		VShaderFile.open(Shader->VertexPath);
		
		std::stringstream VShaderStream;
		VShaderStream << VShaderFile.rdbuf();
		VShaderFile.close();
		VertexCode = VShaderStream.str();

		if (VShaderFile.fail())
			print("ERROR::VERTEX SHADER::FILE_NOT_SUCCESFULLY_READ : %s", VertexShaderFilename.c_str());
	}

	// >> FRAGMENT
	string FragmentCode;
	{
		Shader->FragmentPath = Paths::Shaders + FragmentShaderFilename + Paths::ShaderFileExtension;
		std::ifstream FShaderFile;
		FShaderFile.open(Shader->FragmentPath);
		
		std::stringstream FShaderStream;
		FShaderStream << FShaderFile.rdbuf();
		FShaderFile.close();
		FragmentCode = FShaderStream.str();

		if (FShaderFile.fail())
			print("ERROR::FRAGMENT SHADER::FILE_NOT_SUCCESFULLY_READ : %s", FragmentShaderFilename.c_str());
	}

	// > COMPILE SHADERS
	// >> VERTEX
	unsigned int Vertex;
	{
		const char* VShaderCode = VertexCode.c_str();

		Vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(Vertex, 1, &VShaderCode, nullptr);
		glCompileShader(Vertex);
		Problem = Problem || CheckShaderCompileErrors(Shader, "VERTEX", Vertex);
	}

	// >> FRAGMENT
	unsigned int Fragment;
	{
		const char* FShaderCode = FragmentCode.c_str();

		Fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(Fragment, 1, &FShaderCode, nullptr);
		glCompileShader(Fragment);
		Problem = Problem || CheckShaderCompileErrors(Shader, "FRAGMENT", Fragment);
	}

	// > ATTACH SHADERS TO PROGRAM
	Shader->GLProgramID = glCreateProgram();
	glAttachShader(Shader->GLProgramID, Vertex);
	glAttachShader(Shader->GLProgramID, Fragment);


	// > LOAD, COMPILE AND ATTACH OPTIONAL SHADERS 
	// >> GEOMETRY
	if (BuildGeometryShader)
	{
		// >>> LOAD
		string GeometryCode;
		{
			Shader->GeometryPath = Paths::Shaders + GeometryShaderFilename + Paths::ShaderFileExtension;
			std::ifstream GShaderFile;

			GShaderFile.open(Shader->GeometryPath);
			std::stringstream GShaderStream;
			
			GShaderStream << GShaderFile.rdbuf();
			GShaderFile.close();
			GeometryCode = GShaderStream.str();

			if (GShaderFile.fail())
				print("ERROR::GEOMETRY SHADER::FILE_NOT_SUCCESFULLY_READ : %s", GeometryShaderFilename.c_str());
		}

		// >>> COMPILE
		unsigned int Geometry;
		{
			const char* GShaderCode = GeometryCode.c_str();

			Geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(Geometry, 1, &GShaderCode, nullptr);
			glCompileShader(Geometry);
			Problem = Problem || CheckShaderCompileErrors(Shader, "GEOMETRY", Geometry);
		}

		// >>> ATTACH
		glAttachShader(Shader->GLProgramID, Geometry);
		OptionalShaders[0] = Geometry;
		OptionalShadersCount++;
	}


	// > LINK PROGRAM
	glLinkProgram(Shader->GLProgramID);
	Problem = Problem || CheckShaderCompileErrors(Shader, "PROGRAM", Shader->GLProgramID);


	// > DELETE SHADERS
	// ... as they're linked into our program now and no longer necessary
	glDeleteShader(Vertex);
	glDeleteShader(Fragment);

	for (int I = 0; I < OptionalShadersCount; I++)
		glDeleteShader(OptionalShaders[I]);


	// > ASSERT AND RETURN
	//if(Problem)
	// assert(false);

	return Shader;
}

RShader* CreateShaderProgram(string Name, const string VertexShaderFilename, const string FragmentShaderFilename)
{
	return CreateShaderProgram(Name, VertexShaderFilename, "", FragmentShaderFilename);
}
