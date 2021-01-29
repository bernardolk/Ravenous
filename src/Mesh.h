#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

struct Mesh {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    GLenum render_method;
};


// prototypes
GLData setup_gl_data_for_mesh(Mesh* mesh);
GLData setup_gl_data_for_lines(Vertex* vertices, size_t size);
std::vector<Vertex> construct_cylinder(float radius, float half_lenght, int slices); 



GLData setup_gl_data_for_mesh(Mesh* mesh)
{
   GLData gl_data;

   // create buffers/arrays
   glGenVertexArrays(1, &gl_data.VAO);
   glGenBuffers     (1, &gl_data.VBO);
   glGenBuffers     (1, &gl_data.EBO);

   // load data into vertex buffers
   glBindVertexArray(gl_data.VAO);
   glBindBuffer(GL_ARRAY_BUFFER, gl_data.VBO);
   glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex), &(mesh->vertices[0]), GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_data.EBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), &(mesh->indices[0]), GL_STATIC_DRAW);

   // set the vertex attribute pointers
   // vertex positions
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
   // vertex normals
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
   // vertex texture coords
   glEnableVertexAttribArray(2);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
   // vertex tangent
   glEnableVertexAttribArray(3);
   glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
   // vertex bitangent
   glEnableVertexAttribArray(4);
   glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

   glBindVertexArray(0);

   return gl_data;
}

GLData setup_gl_data_for_lines(Vertex* vertices, size_t size) 
{
   GLData gl_data;

   // create buffers/arrays
   glGenVertexArrays(1, &gl_data.VAO);
   glGenBuffers     (1, &gl_data.VBO);

   // load data into vertex buffers
   glBindVertexArray(gl_data.VAO);
   glBindBuffer(GL_ARRAY_BUFFER, gl_data.VBO);
   glBufferData(GL_ARRAY_BUFFER, size * sizeof(Vertex), vertices, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);

   return gl_data;
} 



std::vector<Vertex> construct_cylinder(float radius, float half_lenght, int slices) 
{
   std::vector<Vertex> vertices;
   for(int i=0; i < slices; i++) {
      float theta = ((float)i) * 2.0 * PI * (1.0 / slices);
      float nextTheta = ((float)i+1) * 2.0 * PI * (1.0 / slices);
      // vertex at middle of end  
      Vertex v = Vertex{glm::vec3(0.0), glm::vec3(half_lenght), glm::vec3(0.0)};
      vertices.push_back(v);
      //vertices at edges of circle 
      v = Vertex{glm::vec3(radius*cos(theta), half_lenght, radius*sin(theta))};
      vertices.push_back(v);
      v = Vertex{glm::vec3(radius*cos(nextTheta), half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      // the same vertices at the bottom of the cylinder (half face)
      v = Vertex{glm::vec3(radius*cos(nextTheta), -half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      v = Vertex{glm::vec3(radius*cos(theta), -half_lenght, radius*sin(theta))};
      vertices.push_back(v);
      // other half face
      v = Vertex{glm::vec3(radius*cos(theta), half_lenght, radius*sin(theta))};
      vertices.push_back(v);
      v = Vertex{glm::vec3(radius*cos(nextTheta), half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      // back from the middle
      v = Vertex{glm::vec3(radius*cos(nextTheta), -half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      v = Vertex{glm::vec3(0.0, -half_lenght, 0.0)};
      vertices.push_back(v);
      // roundabout
      v = Vertex{glm::vec3(radius*cos(theta), -half_lenght, radius*sin(theta))};
      vertices.push_back(v);
      v = Vertex{glm::vec3(radius*cos(nextTheta), -half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      v = Vertex{glm::vec3(0.0, -half_lenght, 0.0)};
      vertices.push_back(v);
   }

   return vertices;
}