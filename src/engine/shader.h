#pragma once

struct Shader {
    unsigned int  gl_programId;
    std::string   name;
    std::string   vertex_path;
    std::string   geometry_path;
    std::string   fragment_path;

    void use();

   void setBool   (const std::string & name, bool value) const;
   void setInt    (const std::string & name, int value) const;
   void setFloat  (const std::string & name, float value) const;
   void setFloat2 (const std::string & name, float value0, float value1) const;
   void setFloat2 (const std::string & name, vec2 vec) const;
   void setFloat3 (const std::string & name, float value0, float value1, float value2) const;
   void setFloat3 (const std::string & name, vec3 vec) const;
   void setFloat4 (const std::string & name, float value0, float value1, float value2, float value3) const;
   void setFloat4 (const std::string & name, glm::vec4 vec) const;
   void setMatrix4(const std::string & name, glm::mat4 mat) const;
};

bool check_shader_compile_errors(Shader* shader, std::string type, unsigned int id);

Shader* create_shader_program(
   std::string        name, 
   const std::string  vertex_shader_filename,
   const std::string  geometry_shader_filename, 
   const std::string  fragment_shader_filename);

Shader* create_shader_program(std::string name, const std::string  vertex_shader_filename, const std::string  fragment_shader_filename);