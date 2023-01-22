#pragma once

struct Shader
{
	unsigned int gl_program_id;
	std::string name;
	std::string vertex_path;
	std::string geometry_path;
	std::string fragment_path;

	void Use();

	void SetBool(const std::string& name, bool value) const;
	void SetInt(const std::string& name, int value) const;
	void SetFloat(const std::string& name, float value) const;
	void SetFloat2(const std::string& name, float value0, float value1) const;
	void SetFloat2(const std::string& name, vec2 vec) const;
	void SetFloat3(const std::string& name, float value0, float value1, float value2) const;
	void SetFloat3(const std::string& name, vec3 vec) const;
	void SetFloat4(const std::string& name, float value0, float value1, float value2, float value3) const;
	void SetFloat4(const std::string& name, vec4 vec) const;
	void SetMatrix4(const std::string& name, mat4 mat) const;
};

extern std::map<std::string, Shader*> ShaderCatalogue;

bool check_shader_compile_errors(Shader* shader, std::string type, unsigned int id);

Shader* create_shader_program(
	std::string name,
	std::string vertex_shader_filename,
	std::string geometry_shader_filename,
	std::string fragment_shader_filename
);

Shader* create_shader_program(std::string name, std::string vertex_shader_filename, std::string fragment_shader_filename);
