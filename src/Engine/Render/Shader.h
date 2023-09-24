#pragma once

#include "engine/core/core.h"

struct RShader
{
	unsigned int gl_program_id;
	string name;
	string vertex_path;
	string geometry_path;
	string fragment_path;

	void Use();

	void SetBool(const string& name, bool value) const;
	void SetInt(const string& name, int value) const;
	void SetFloat(const string& name, float value) const;
	void SetFloat2(const string& name, float value0, float value1) const;
	void SetFloat2(const string& name, vec2 vec) const;
	void SetFloat3(const string& name, float value0, float value1, float value2) const;
	void SetFloat3(const string& name, vec3 vec) const;
	void SetFloat4(const string& name, float value0, float value1, float value2, float value3) const;
	void SetFloat4(const string& name, vec4 vec) const;
	void SetMatrix4(const string& name, mat4 mat) const;
};

extern map<string, RShader*> ShaderCatalogue;

bool CheckShaderCompileErrors(RShader* shader, string type, unsigned int id);

RShader* CreateShaderProgram(
	string name,
	string vertex_shader_filename,
	string geometry_shader_filename,
	string fragment_shader_filename
);

RShader* CreateShaderProgram(string name, string vertex_shader_filename, string fragment_shader_filename);
