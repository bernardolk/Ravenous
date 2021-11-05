#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 tex_coords;
    vec3 tangent;
    vec3 bitangent;

    Vertex operator*(glm::mat4 mat)
    {
       // should do something for tex_coords ???

       vec3 n_position   = vec3(mat * vec4(position,  1.f));
       vec3 n_normal     = vec3(mat * vec4(normal,    1.f));
       vec3 n_tangent    = vec3(mat * vec4(tangent,   1.f));
       vec3 n_bitangent  = vec3(mat * vec4(bitangent, 1.f));

      return Vertex{n_position, n_normal, tex_coords, n_tangent, n_bitangent};
    };

    Vertex& operator*=(glm::mat4 mat)
    {
       // should do something for tex_coords ???

       position   = vec3(mat * vec4(position,  1.f));
       normal     = vec3(mat * vec4(normal,    1.f));
       tangent    = vec3(mat * vec4(tangent,   1.f));
       bitangent  = vec3(mat * vec4(bitangent, 1.f));

      return *this;
    };
};

struct Texture {
    unsigned int id;
    string type;
    string path;
    string name;
};

struct Mesh {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    GLenum render_method;
    GLData gl_data;
    string name;
    FILETIME last_written;

   void setup_gl_data()
   {
      // to avoid a pretty bad rendering issue
      assert(indices.size() > 0);

      GLData new_gl_data;

      // create buffers/arrays
      glGenVertexArrays(1, &new_gl_data.VAO);
      glGenBuffers     (1, &new_gl_data.VBO);
      glGenBuffers     (1, &new_gl_data.EBO);

      // load data into vertex buffers
      glBindVertexArray(new_gl_data.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, new_gl_data.VBO);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &(vertices[0]), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_gl_data.EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &(indices[0]), GL_STATIC_DRAW);

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

      gl_data = new_gl_data;
      return;
   }

   // This will only create the buffer and set the attribute pointers
   void setup_gl_buffers()
   {
       GLData new_gl_data;

      // create buffers/arrays
      glGenVertexArrays(1, &new_gl_data.VAO);
      glGenBuffers     (1, &new_gl_data.VBO);
      glGenBuffers     (1, &new_gl_data.EBO);  

      gl_data = new_gl_data;
      return;
   }

   void send_data_to_gl_buffer()
   {
      // load data into vertex buffers
      glBindVertexArray(gl_data.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, gl_data.VBO);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &(vertices[0]), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_data.EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &(indices[0]), GL_STATIC_DRAW);

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
   }


   BoundingBox compute_bounding_box()
   {
      // This returns a bounding box that contains the mesh
      // Vertices of the bounding box do not necessarely match vertices in the mesh
      // So, this does NOT return the min/max vertices of the mesh in axial direction
      // (support points)

      vec3 max_d = vec3(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);
      vec3 min_d = vec3(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT);

      float maxx, minx, maxy, miny, maxz, minz;

      for (int i = 0; i < vertices.size(); i++)
      {
         vec3 vertex = vertices[i].position;
         float dotx = glm::dot(vertex, vec3(1,0,0));
         float doty = glm::dot(vertex, vec3(0,1,0));
         float dotz = glm::dot(vertex, vec3(0,0,1));

         if(dotx < min_d.x)
         {
            minx    = vertex.x;
            min_d.x = dotx;
         }
         if(dotx > max_d.x)
         {
            maxx    = vertex.x;
            max_d.x = dotx;
         }

         if(doty < min_d.y)
         {
            miny    = vertex.y;
            min_d.y = doty;
         }
         if(doty > max_d.y)
         {
            maxy    = vertex.y;
            max_d.y = doty;
         }

         if(dotz < min_d.z)
         {
            minz    = vertex.z;
            min_d.z = dotz;
         }
         if(dotz > max_d.z)
         {
            maxz    = vertex.z;
            max_d.z = dotz;
         }
      }

      BoundingBox bb;
      bb.set(vec3(minx, miny, minz), vec3(maxx, maxy, maxz));
      return bb;
   }
};


// prototypes
GLData setup_gl_data_for_lines(Vertex* vertices, size_t size);
std::vector<Vertex> construct_cylinder(float radius, float half_lenght, int slices); 



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
      Vertex v = Vertex{vec3(0.0), vec3(half_lenght), vec3(0.0)};
      vertices.push_back(v);
      //vertices at edges of circle 
      v = Vertex{vec3(radius*cos(theta), half_lenght, radius*sin(theta))};
      vertices.push_back(v);
      v = Vertex{vec3(radius*cos(nextTheta), half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      // the same vertices at the bottom of the cylinder (half face)
      v = Vertex{vec3(radius*cos(nextTheta), -half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      v = Vertex{vec3(radius*cos(theta), -half_lenght, radius*sin(theta))};
      vertices.push_back(v);
      // other half face
      v = Vertex{vec3(radius*cos(theta), half_lenght, radius*sin(theta))};
      vertices.push_back(v);
      v = Vertex{vec3(radius*cos(nextTheta), half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      // back from the middle
      v = Vertex{vec3(radius*cos(nextTheta), -half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      v = Vertex{vec3(0.0, -half_lenght, 0.0)};
      vertices.push_back(v);
      // roundabout
      v = Vertex{vec3(radius*cos(theta), -half_lenght, radius*sin(theta))};
      vertices.push_back(v);
      v = Vertex{vec3(radius*cos(nextTheta), -half_lenght, radius*sin(nextTheta))};
      vertices.push_back(v);
      v = Vertex{vec3(0.0, -half_lenght, 0.0)};
      vertices.push_back(v);
   }

   return vertices;
}

