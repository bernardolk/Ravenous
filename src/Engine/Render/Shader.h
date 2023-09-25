#pragma once

#include "engine/core/core.h"

struct RShader
{
	unsigned int GLProgramID;
	string Name;
	string VertexPath;
	string GeometryPath;
	string FragmentPath;

	void Use();

	void SetBool(const string& Name, bool Value) const;
	void SetInt(const string& Name, int Value) const;
	void SetFloat(const string& Name, float Value) const;
	void SetFloat2(const string& Name, float Value0, float Value1) const;
	void SetFloat2(const string& Name, vec2 Vec) const;
	void SetFloat3(const string& Name, float Value0, float Value1, float Value2) const;
	void SetFloat3(const string& Name, vec3 Vec) const;
	void SetFloat4(const string& Name, float Value0, float Value1, float Value2, float Value3) const;
	void SetFloat4(const string& Name, vec4 Vec) const;
	void SetMatrix4(const string& Name, mat4 Mat) const;
};

extern map<string, RShader*> ShaderCatalogue;

bool CheckShaderCompileErrors(RShader* Shader, string Type, unsigned int Id);

RShader* CreateShaderProgram(
	string Name,
	string VertexShaderFilename,
	string GeometryShaderFilename,
	string FragmentShaderFilename
);

RShader* CreateShaderProgram(string Name, string VertexShaderFilename, string FragmentShaderFilename);
