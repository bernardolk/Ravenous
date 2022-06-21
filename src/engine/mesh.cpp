 
 #include <rvn_macros.h>
 #include <map>
 #include <string>
 #include <engine/core/rvn_types.h>
 #include <engine/mesh.h>
 
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
   glGenBuffers     (1, &this->gl_dataVBO);
   glGenBuffers     (1, &this->gl_dataEBO);  
}

void Mesh::send_data_to_gl_buffer()
{
   // load data into vertex buffers
   glBindVertexArray (this->gl_data.VAO);
   glBindBuffer      (GL_ARRAY_BUFFER, this->gl_data.VBO);
   glBufferData      (GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &(this->vertices[0]), GL_STATIC_DRAW);
   glBindBuffer      (GL_ELEMENT_ARRAY_BUFFER, this->gl_data.EBO);
   glBufferData      (GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &(this->indices[0]), GL_STATIC_DRAW);

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