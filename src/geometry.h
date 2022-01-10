void create_boilerplate_geometry()
{
   //TEXT
   GLData text_gl_data;
	glGenVertexArrays(1, &text_gl_data.VAO);
	glGenBuffers(1, &text_gl_data.VBO);
	glBindVertexArray(text_gl_data.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, text_gl_data.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

   Mesh* text_mesh = new Mesh();
   text_mesh->name = "text";
   text_mesh->gl_data = text_gl_data;
   Geometry_Catalogue.insert({text_mesh->name, text_mesh});

   // AABB
   vector<Vertex> aabb_vertex_vec = {
      // bottom
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.5f, 0.5f)},   //0
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 0.5f)},   //1
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 1.0f)},   //2
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.5f, 1.0f)},   //3
      // top   
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(0.0f, 1.0f, 0.0f),    vec2(0.5f, 0.5f)},   //4
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(0.0f, 1.0f, 0.0f),    vec2(1.0f, 0.5f)},   //5
      Vertex{vec3(1.0f, 1.0f, 1.0f),   vec3(0.0f, 1.0f, 0.0f),    vec2(1.0f, 1.0f)},   //6
      Vertex{vec3(1.0f, 1.0f, 0.0f),   vec3(0.0f, 1.0f, 0.0f),    vec2(0.5f, 1.0f)},   //7
      // front       
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.0f, 0.0f)},   //8
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.5f, 0.0f)},   //9
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.0f, 0.5f)},   //10
      Vertex{vec3(1.0f, 1.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.5f, 0.5f)},   //11
      // back
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),    vec2(0.0f, 0.0f)},   //12
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),    vec2(0.0f, 0.5f)},   //13
      Vertex{vec3(1.0f, 1.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),    vec2(0.5f, 0.5f)},   //14
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),    vec2(0.5f, 0.0f)},   //15
      // left
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.0f, 0.0f)},   //16
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.5f, 0.0f)},   //17
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.0f, 0.5f)},   //18
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.5f, 0.5f)},   //19
      // right
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(1.0f, 0.0f, 0.0f),    vec2(0.0f, 0.0f)},   //20
      Vertex{vec3(1.0f, 1.0f, 0.0f),   vec3(1.0f, 0.0f, 0.0f),    vec2(0.0f, 0.5f)},   //21
      Vertex{vec3(1.0f, 1.0f, 1.0f),   vec3(1.0f, 0.0f, 0.0f),    vec2(0.5f, 0.5f)},   //22
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(1.0f, 0.0f, 0.0f),    vec2(0.5f, 0.0f)},   //23
   }; 

   // computes AABB tangent and bitangent vectors
   For(6)   // for every face
   {
      // First Triangle
      {
         vec3 edge1      = aabb_vertex_vec[i * 4 + 1].position    - aabb_vertex_vec[i * 4].position;
         vec3 edge2      = aabb_vertex_vec[i * 4 + 2].position    - aabb_vertex_vec[i * 4].position;
         vec2 deltaUV1   = aabb_vertex_vec[i * 4 + 1].tex_coords  - aabb_vertex_vec[i * 4].tex_coords;
         vec2 deltaUV2   = aabb_vertex_vec[i * 4 + 2].tex_coords  - aabb_vertex_vec[i * 4].tex_coords;

         vec3 tangent, bitangent;
         float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
         tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
         tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
         tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

         bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
         bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
         bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

         aabb_vertex_vec[i * 4 + 0].tangent   = tangent;
         aabb_vertex_vec[i * 4 + 0].bitangent = bitangent;
         aabb_vertex_vec[i * 4 + 1].tangent   = tangent;
         aabb_vertex_vec[i * 4 + 1].bitangent = bitangent;
         aabb_vertex_vec[i * 4 + 2].tangent   = tangent;
         aabb_vertex_vec[i * 4 + 2].bitangent = bitangent;
      }

      // Second Triangle
      {
         vec3 edge1      = aabb_vertex_vec[i * 4 + 1].position    - aabb_vertex_vec[i * 4].position;
         vec3 edge2      = aabb_vertex_vec[i * 4 + 2].position    - aabb_vertex_vec[i * 4].position;
         vec2 deltaUV1   = aabb_vertex_vec[i * 4 + 1].tex_coords  - aabb_vertex_vec[i * 4].tex_coords;
         vec2 deltaUV2   = aabb_vertex_vec[i * 4 + 2].tex_coords  - aabb_vertex_vec[i * 4].tex_coords;

         vec3 tangent, bitangent;
         float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
         tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
         tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
         tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

         bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
         bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
         bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

         aabb_vertex_vec[i * 4 + 0].tangent   = tangent;
         aabb_vertex_vec[i * 4 + 0].bitangent = bitangent;
         aabb_vertex_vec[i * 4 + 1].tangent   = tangent;
         aabb_vertex_vec[i * 4 + 1].bitangent = bitangent;
         aabb_vertex_vec[i * 4 + 2].tangent   = tangent;
         aabb_vertex_vec[i * 4 + 2].bitangent = bitangent;
      }
   }

   
   vector<u32> aabb_vertex_indices = 
   { 
      0, 1, 2, 2, 3, 0,          // bottom face
      16, 17, 18, 19, 18, 17,    // left face
      12, 13, 14, 14, 15, 12,    // back face
      20, 21, 22, 22, 23, 20,    // right face
      8, 9, 10, 11, 10, 9,       // front face
      4, 5, 6, 6, 7, 4           // top face
   };

   auto aabb_mesh                = new Mesh();
   aabb_mesh->name               = "aabb";
   aabb_mesh->vertices           = aabb_vertex_vec;
   aabb_mesh->indices            = aabb_vertex_indices;
   aabb_mesh->render_method      = GL_TRIANGLES;
   aabb_mesh->setup_gl_data();
   Geometry_Catalogue.insert({aabb_mesh->name, aabb_mesh});

   auto world_cell_mesh             = new Mesh();
   world_cell_mesh->name            = "world cell";
   world_cell_mesh->vertices        = aabb_vertex_vec;
   world_cell_mesh->indices         = aabb_vertex_indices;
   world_cell_mesh->render_method   = GL_TRIANGLES;
   world_cell_mesh->setup_gl_data();
   Geometry_Catalogue.insert({world_cell_mesh->name, world_cell_mesh});

   // SLOPE
   // with Z coming at the screen, X to the right, slope starts at x=0 high and goes low on x=1
   vector<Vertex> slope_vertex_vec = {
      // bottom
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.5f, 0.5f)},   //0
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 0.5f)},   //1
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(1.0f, 1.0f)},   //2
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(0.0f, -1.0f, 0.0f),   vec2(0.5f, 1.0f)},   //3
      // right   
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.5f, 0.5f, 0.0f),    vec2(1.0f, 0.5f)},   //4
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.5f, 0.5f, 0.0f),    vec2(0.5f, 0.5f)},   //5
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(0.5f, 0.5f, 0.0f),    vec2(1.0f, 1.0f)},   //6
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(0.5f, 0.5f, 0.0f),    vec2(0.5f, 1.0f)},   //7
      // front       
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.0f, 0.0f)},   //8
      Vertex{vec3(1.0f, 0.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.5f, 0.0f)},   //9
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(0.0f, 0.0f, 1.0f),    vec2(0.0f, 0.5f)},   //10
      // back
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),   vec2(0.0f, 0.0f)},   //11
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),   vec2(0.0f, 0.5f)},   //12
      Vertex{vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f),   vec2(0.5f, 0.0f)},   //13
      // left
      Vertex{vec3(0.0f, 0.0f, 0.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.0f, 0.0f)},   //14
      Vertex{vec3(0.0f, 0.0f, 1.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.5f, 0.0f)},   //15
      Vertex{vec3(0.0f, 1.0f, 0.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.0f, 0.5f)},   //16
      Vertex{vec3(0.0f, 1.0f, 1.0f),   vec3(-1.0f, 0.0f, 0.0f),   vec2(0.5f, 0.5f)},   //17
   };

   vector<u32> slope_vertex_indices = 
   { 
      0, 1, 2, 2, 3, 0,          // bottom face
      8, 9, 10,                  // front
      11, 12, 13,                // back
      14, 15, 16, 17, 16, 15,    // left face
      4, 5, 6, 6, 7, 4           // right face (slope)
   };

   auto slope_mesh               = new Mesh();
   slope_mesh->name              = "slope";
   slope_mesh->vertices          = slope_vertex_vec;
   slope_mesh->indices           = slope_vertex_indices;
   slope_mesh->render_method     = GL_TRIANGLES;
   slope_mesh->setup_gl_data();
   Geometry_Catalogue.insert({slope_mesh->name, slope_mesh});

   // PLANE VBO
   vector<Vertex> planeVertices = {
      // positions            // normals         // texcoords
      Vertex{vec3{25.0f, -0.5f, 25.0f},   vec3{0.0f, 1.0f, 0.0f},   vec2{25.0f, 0.0f}},
      Vertex{vec3{-25.0f, -0.5f, 25.0f},  vec3{0.0f, 1.0f, 0.0f},   vec2{0.0f, 0.0f}},
      Vertex{vec3{-25.0f, -0.5f, -25.0f}, vec3{0.0f, 1.0f, 0.0f},   vec2{0.0f, 25.0f}},

      Vertex{vec3{25.0f, -0.5f,  25.0f},  vec3{0.0f, 1.0f, 0.0f},  vec2{25.0f,  0.0f}},
      Vertex{vec3{-25.0f, -0.5f, -25.0f}, vec3{0.0f, 1.0f, 0.0f},  vec2{ 0.0f, 25.0f}},
      Vertex{vec3{25.0f, -0.5f, -25.0f},  vec3{0.0f, 1.0f, 0.0f},  vec2{25.0f, 10.0f}}
   };
   auto plane_mesh               = new Mesh();
   plane_mesh->name              = "plane";
   plane_mesh->vertices          = planeVertices;
   plane_mesh->indices           = {0, 1, 2, 3, 4, 5};
   plane_mesh->render_method     = GL_TRIANGLES;
   plane_mesh->setup_gl_data();
   Geometry_Catalogue.insert({plane_mesh->name, plane_mesh});


   // QUAD VBO
   vector<Vertex> quad_vertex_vec = {
      Vertex{vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 0.0f)},
      Vertex{vec3(1.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 0.0f)},
      Vertex{vec3(1.0f, 1.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 1.0f)},
      Vertex{vec3(0.0f, 1.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 1.0f)}
   };
   // QUAD EBO
   vector<u32> quad_vertex_indices = { 0, 1, 2, 2, 3, 0 };


   // LINE (position is updated directly into VBO)
   /*vector<Vertex> line_vertex_vec = {
         Vertex{vec3(1, 1, 0)},
         Vertex{vec3(1, 1, 1)},
         Vertex{vec3(0, 1, 1)},
         Vertex{vec3(0, 1, 0)}
   };*/

   Mesh* quad_mesh            = new Mesh();
   quad_mesh->name            = "quad";
   quad_mesh->vertices        = quad_vertex_vec;
   quad_mesh->indices         = quad_vertex_indices;
   quad_mesh->render_method   = GL_TRIANGLES;
   quad_mesh->setup_gl_data();
   Geometry_Catalogue.insert({quad_mesh->name, quad_mesh});

 // QUAD HORIZONTAL
   vector<Vertex> quad_horizontal_vertex_vec = {
      Vertex{vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 0.0f)},
      Vertex{vec3(1.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 0.0f)},
      Vertex{vec3(1.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, 1.0f),vec2(1.0f, 1.0f)},
      Vertex{vec3(0.0f, 0.0f, 1.0f),vec3(0.0f, 0.0f, 1.0f),vec2(0.0f, 1.0f)}
   };

   Mesh* quad_horizontal_mesh             = new Mesh();
   quad_horizontal_mesh->name             = "quad_horizontal";
   quad_horizontal_mesh->vertices         = quad_horizontal_vertex_vec;
   quad_horizontal_mesh->indices          = quad_vertex_indices;
   quad_horizontal_mesh->render_method    = GL_TRIANGLES;
   quad_horizontal_mesh->setup_gl_data();
   Geometry_Catalogue.insert({quad_horizontal_mesh->name, quad_horizontal_mesh});

   // TRIGGER
   auto trigger_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "player_cylinder");
   Geometry_Catalogue.insert({trigger_mesh->name, trigger_mesh});

   // PLAYER CAPSULE
   load_wavefront_obj_as_mesh(MODELS_PATH, "capsule");

   // LIGHTBULB
   auto lightbulb_mesh = load_wavefront_obj_as_mesh(MODELS_PATH, "lightbulb");
}