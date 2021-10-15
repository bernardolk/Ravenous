
// -----------------------------------------------
//    GJK - Gilbert–Johnson–Keerthi Algorithm
// -----------------------------------------------
// Utilizes the fact that the Minkowski difference's shape of two meshes will contain
// the origin if they are intersecting, and it won't in case not.
// 
// props to blog.winter.dev for the excellent explanation of the algorithm and code examples.
//
// Here, we define data structures to be used by the algorithm and auxiliary functions for the
// main looping function. Each iteration we find a point in the minkowski difference that is
// a candidate for composing a simplex (simplest geom. form in N-dimensions [line, triangle,
// tetrahedron, ...]) that will enclose the origin and check if we should change direction or
// discard one or another point inside the simplex by computing where the origin is in relation
// to each face/region delimited by the current simplex. The direction mentioned is the direction
// in which we will try picking the furthest point in the minkowski's difference shape.


// ------------------
// > SIMPLEX
// ------------------
struct Simplex {
	private:
		std::array<vec3, 4> points;
		uint size;

	public:
		Simplex()
			: points({ 0, 0, 0, 0 })
			, size(0)
		{}

		Simplex& operator=(std::initializer_list<vec3> list) {
			for (auto v = list.begin(); v != list.end(); v++) {
				points[std::distance(list.begin(), v)] = *v;
			}
			size = list.size();

			return *this;
		}

		void push_front(vec3 point) {
			points = { point, points[0], points[1], points[2] };
			size = std::min(size + 1, 4u);
		}

		vec3& operator[](uint i) { return points[i]; }
		uint size() const { return size; }
		auto begin() const { return points.begin(); }
		auto end()   const { return points.end() - (4 - size); }
};

// ------------------
// > GJK_Iteration
// ------------------
struct GJK_Iteration {
   Simplex simplex;
   vec3 direction;
   bool finished = false;
};

// ------------------
// > GJK_Point
// ------------------
struct GJK_Point {
   vec3 point;
   bool empty;
};

// --------------------
// > UTILITY FUNCTIONS
// --------------------

inline
bool CL_same_general_direction(vec3 a, vec3 b)
{
   return glm::dot(a, b) > 0;
}


// ------------------------
// > GJK SUPPORT FUNCTIONS
// ------------------------

GJK_Point CL_find_furthest_vertex(Mesh collision_mesh, vec3 direction):
{
	// linearly scan the CollisionMesh doing dot products with the vertices and storing the one with max value, then return it

   float max_inner_p = MIN_FLOAT;
   vec3 furthest_vertex;

   for (i = 0; i < collision_mesh->vertices.size(); i++)
   {
      auto vertex = vertices[i];
      vec3 position = vertex[0];
      float inner_p = glm::dot(position, direction);
      if(inner_p > max_inner_p)
      {
         max_inner_p = inner_p;
         furthest_vertex = position;
      }
   }

   return GJK_Point{furthest_vertex, max_inner_p == MIN_FLOAT};
}


GJK_Point CL_get_support_point(Mesh collision_mesh_A, Mesh collision_mesh_A, vec3 direction)
{
   // Gets a support point in the minkowski difference of both meshes, in the direction supplied.

   GJK_Point point_a = CL_find_furthest_vertex(collision_mesh_A, direction);
   GJK_Point point_b = CL_find_furthest_vertex(collision_mesh_B, -1.f * direction);

   if (point_a.empty || point_b.empty)
      return point_a;

   return GJK_Point{point_a.vertex - point_b.vertex, false};
}

// -----------------------
// > UPDATE SIMPLEX CALLS
// -----------------------

GJK_Iteration CL_update_line_simplex(GJK_Iteration gjk)
{
   // auto points = gjk.simplex;
   // auto direction = gjk.direction;

   vec3 a = simplex[0];
   vec3 b = simplex[1];

   vec3 ab = b - a;
   vec3 ao =   - a;

   if (CL_same_general_direction(ab, ao))
      return GJK_Iteration{gjk.simplex, direction = cross(ab, ao, ab)};

   else
      return GJK_Iteration{simplex = { a }, direction = ao};
}


GJK_Iteration CL_update_triangle_simplex(GJK_Iteration gjk)
{
   // auto points = gjk.simplex;
   //auto direction = gjk.direction;

   vec3 a = simplex[0];
   vec3 b = simplex[1];
   vec3 c = simplex[2];

   vec3 ab = b - a;
   vec3 ac = c - a;
   vec3 ao =   - a;
   
   vec3 abc = glm::cross(ab, ac);

   if(CL_same_general_direction(glm::cross(abc, ac), ao))
   {
      if(CL_same_general_direction(ac, ao))
         return GJK_Iteration{simplex = { a, c }, direction = cross(ac, ao, ac)};

      else
         return CL_update_line_simplex(GJK_Iteration{simplex = { a, b }, gjk.direction});
   }

   else
   {
      if(CL_same_general_direction(glm::cross(ab, abc), ao))
         return CL_update_line_simplex(GJK_Iteration{simplex = {a, b}, gjk.direction});

      else
      {
         if(CL_same_general_direction(abc, ao))
            return GJK_Iteration{simplex = gjk.simplex, direction = abc};
         else
            return GJK_Iteration{simplex = {a, c, b}, direction = -1.f * abc};
      }
   }
}


GJK_Iteration CL_update_tetrahedron_simplex(GJK_Iteration gjk)
{
   vec3 a = gjk.simplex[0];
	vec3 b = gjk.simplex[1];
	vec3 c = gjk.simplex[2];
	vec3 d = gjk.simplex[3];

	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 ad = d - a;
	vec3 ao =   - a;
 
	vec3 abc = glm::cross(ab, ac);
	vec3 acd = glm::cross(ac, ad);
	vec3 adb = glm::cross(ad, ab);

   // check if mikowski's diff origin is pointing towards tetrahedron normal faces
   // (it shouldn't, since the origin should be contained in the shape and point down not up like the inclined faces's normals)

   if(CL_same_general_direction(abc, ao))
   {
      return CL_update_triangle_simplex(GJK_Iteration{simplex = {a, b, c}, direction = gjk.direction});
   }

   if(CL_same_general_direction(acd, ao))
   {
      return CL_update_triangle_simplex(GJK_Iteration{simplex = {a, c, d}, direction = gjk.direction});
   }

   if(CL_same_general_direction(adb, ao))
   {
      return CL_update_triangle_simplex(GJK_Iteration{simplex = {a, d, b}, direction = gjk.direction});
   }

   // was not the case, found collision
   return GJK_Iteration{points = {a, b, c, d}, direction = gjk.direction, true};
}


GJK_Iteration CL_update_simplex_and_direction(GJK_Iteration gjk)
{
	switch(gjk.simplex.size())
   {
      case 2:
         return CL_update_line_simplex(gjk);
      case 3:
         return CL_update_triangle_simplex(gjk);
      case 4:
         return CL_update_tetrahedron_simplex(gjk);
   }

   // something went wrong
   assert(false);
}


// ------------------
// > CL_RUN_GJK
// ------------------

bool CL_run_GJK(Entity entity_A, Entity entity_B)
{
   // Runs GJK algorithm
   Mesh collider_A = entity_A.collision_mesh;
   Mesh collider_B = entity_B.collision_mesh;

   GJK_Point support_point = CL_get_support_point(collider_A, collider_B, vec3::unit_x);

   GJK_Iteration gjk;
   gjk.simplex.push_front(support_point);
   gjk.direction = -1.0f * support_point;

   while(true)
   {
      support_point = CL_get_support_point(collider_A, collider_B, vec3::unit_x);

      if(!CL_same_general_direction(support_point, gjk.direction))
         return false;    // no collision

      gjk.simplex.push_front(support_point);

      gjk = CL_update_simplex_and_direction(gjk);

      if(gjk.finished)
         return true;
   }
}