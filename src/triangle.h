struct Triangle {
   vec3 a;
   vec3 b;
   vec3 c;
};

// -----------------------
// > TRIANGLE OPERATIONS
// -----------------------
inline
vec3 get_triangle_normal(Triangle t)
{
   return glm::triangleNormal(t.a, t.b, t.c);
}

inline
vec3 get_barycenter(Triangle t)
{
   auto bx = (t.a.x + t.b.x + t.c.x) / 3;
   auto by = (t.a.y + t.b.y + t.c.y) / 3;
   auto bz = (t.a.z + t.b.z + t.c.z) / 3;
   return vec3(bx, by, bz);
}

inline
bool is_equal(Triangle t1, Triangle t2)
{
   return t1.a == t2.a && t1.b == t2.b && t1.c == t2.c;
}

inline
bool is_valid(Triangle t)
{
   // checks if vertices are not in a single point
   return t.a != t.b && t.a != t.c && t.b != t.c;
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


Triangle get_triangle_for_indexed_mesh(Entity* entity, int triangle_index)
{
   Mesh* mesh = entity->mesh;
   mat4 model = entity->matModel;

   return get_triangle_for_indexed_mesh(mesh, model, triangle_index);
}