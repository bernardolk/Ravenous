#include <rvn_macros.h>
#include <map>
#include <vector>
#include <string>
#include <engine/core/rvn_types.h>
#include <glad/glad.h>
#include <engine/collision/primitives/bounding_box.h>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/triangle.h>
#include <engine/vertex.h>
#include <iostream>
#include <engine/logging.h>
#include <engine/mesh.h>
#include <engine/collision/collision_mesh.h>

/*
   Mesh vertex readme:
   - Vertex count = N_quad_faces * 4
   - Indices count = N_quad_faces * 6 or N_triang_faces * 3
*/


std::map<std::string, Mesh*>              Geometry_Catalogue;
std::map<std::string, CollisionMesh*>     Collision_Geometry_Catalogue;
std::map<std::string, Texture>            Texture_Catalogue;

void Mesh::setup_gl_data()
{
   // to avoid a pretty bad rendering issue
   assert(indices.size() > 0);

   if(gl_data.VAO > 0)
   {
      log(LOG_INFO, "Redundant setup_gl_data call occured.");
      return;
   }

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
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &(indices[0]), GL_STATIC_DRAW);

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
}

// This will only create the buffer and set the attribute pointers
void Mesh::setup_gl_buffers()
{
   glGenVertexArrays(1, &this->gl_data.VAO);
   glGenBuffers     (1, &this->gl_data.VBO);
   glGenBuffers     (1, &this->gl_data.EBO);  
}

void Mesh::send_data_to_gl_buffer()
{
   // load data into vertex buffers
   glBindVertexArray (this->gl_data.VAO);
   glBindBuffer      (GL_ARRAY_BUFFER, this->gl_data.VBO);
   glBufferData      (GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &(this->vertices[0]), GL_STATIC_DRAW);
   glBindBuffer      (GL_ELEMENT_ARRAY_BUFFER, this->gl_data.EBO);

   if (this->indices.size() > 0)
   {
       glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &(this->indices[0]), GL_STATIC_DRAW);
   }
   // @TODO: Do we need to do this every time?
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


BoundingBox Mesh::compute_bounding_box()
{
   // This returns a bounding box that contains the mesh
   // Vertices of the bounding box do not necessarely match vertices in the mesh
   // So, this does NOT return the min/max vertices of the mesh in axial direction
   // (support points)

   vec3 max_d = vec3(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);
   vec3 min_d = vec3(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT);

   float maxx, minx, maxy, miny, maxz, minz;

   for (int i = 0; i < this->vertices.size(); i++)
   {
      vec3 vertex = this->vertices[i].position;
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

void Mesh::compute_tangents_and_bitangents()
{
    // @TODO: This may lead to bugs, we currentyl assume here that faces = 2 triangles each, and while that may hold true with the current loader, that may not remain the case forever.
   For(this->faces_count)
   {
      Vertex v1 = this->vertices[indices[i * 3 + 0]];
      Vertex v2 = this->vertices[indices[i * 3 + 1]];
      Vertex v3 = this->vertices[indices[i * 3 + 2]];

      vec3 edge1     = v2.position   - v1.position;
      vec3 edge2     = v3.position   - v1.position;
      vec2 delta_uv1 = v2.tex_coords - v1.tex_coords;
      vec2 delta_uv2 = v3.tex_coords - v1.tex_coords; 

      float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

      vec3 tangent, bitangent;
      tangent.x     = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
      tangent.y     = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
      tangent.z     = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);

      bitangent.x   = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
      bitangent.y   = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
      bitangent.z   = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);

      this->vertices[indices[i * 3 + 0]].tangent   = tangent;
      this->vertices[indices[i * 3 + 0]].bitangent = bitangent;

      this->vertices[indices[i * 3 + 1]].tangent   = tangent;
      this->vertices[indices[i * 3 + 1]].bitangent = bitangent;

      this->vertices[indices[i * 3 + 2]].tangent   = tangent;
      this->vertices[indices[i * 3 + 2]].bitangent = bitangent;
   }
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

// -----------------------------------------
// > GET TRIANGLE FOR COLLIDER INDEXED MESH
// -----------------------------------------
Triangle get_triangle_for_collider_indexed_mesh(Mesh* mesh, int triangle_index)
{
   auto a_ind = mesh->indices[3 * triangle_index + 0];
   auto b_ind = mesh->indices[3 * triangle_index + 1];
   auto c_ind = mesh->indices[3 * triangle_index + 2];

   auto a = mesh->vertices[a_ind].position;
   auto b = mesh->vertices[b_ind].position;
   auto c = mesh->vertices[c_ind].position;

   return Triangle{a, b, c};
}

Triangle get_triangle_for_collider_indexed_mesh(CollisionMesh* mesh, int triangle_index)
{
   auto a_ind = mesh->indices[3 * triangle_index + 0];
   auto b_ind = mesh->indices[3 * triangle_index + 1];
   auto c_ind = mesh->indices[3 * triangle_index + 2];

   auto a = mesh->vertices[a_ind];
   auto b = mesh->vertices[b_ind];
   auto c = mesh->vertices[c_ind];

   return Triangle{ a, b, c };
}

// --------------------------------
// > GET TRIANGLE FOR INDEXED MESH
// --------------------------------
Triangle get_triangle_for_indexed_mesh(Mesh* mesh, glm::mat4 matModel, int triangle_index)
{
   auto a_ind = mesh->indices[3 * triangle_index + 0];
   auto b_ind = mesh->indices[3 * triangle_index + 1];
   auto c_ind = mesh->indices[3 * triangle_index + 2];

   auto a_vertice = mesh->vertices[a_ind].position;
   auto b_vertice = mesh->vertices[b_ind].position;
   auto c_vertice = mesh->vertices[c_ind].position;

   auto a = matModel * glm::vec4(a_vertice, 1.0);
   auto b = matModel * glm::vec4(b_vertice, 1.0);
   auto c = matModel * glm::vec4(c_vertice, 1.0);

   return Triangle{a, b, c};
}