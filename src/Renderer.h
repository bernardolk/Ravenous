#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <string>
#include <stdio.h>
#include <vector>

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};


unsigned int renderer_setup_skybox(){
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    std::string texture_faces[] = {
        "w:\\assets\\textures\\skybox\\2\\right.jpg",
        "w:\\assets\\textures\\skybox\\2\\left.jpg",
        "w:\\assets\\textures\\skybox\\2\\top.jpg",
        "w:\\assets\\textures\\skybox\\2\\bottom.jpg",
        "w:\\assets\\textures\\skybox\\2\\front.jpg",
        "w:\\assets\\textures\\skybox\\2\\back.jpg",
    };

    int width, height, nrChannels;
    unsigned char* data;
    for(unsigned int i  = 0; i < 6; i++){
        data = stbi_load(texture_faces[i].c_str(), &width, &height, &nrChannels, 0);
        if(data){
            std::cout << "LOADED ONE \n";
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << texture_faces[i] << std::endl;
            stbi_image_free(data);
        }    
    } 

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void render_simple(unsigned int VAO, int triangleCount, Shader shader, float scale, Camera active_camera){
  shader.use();
  shader.setMatrix4("view", active_camera.View4x4);
  shader.setMatrix4("projection", active_camera.Projection4x4);
  glm::mat4 model = glm::scale(mat4identity, glm::vec3(1.0, 1.0, 1.0) * scale); 
  shader.setMatrix4("model", model);
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, triangleCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
