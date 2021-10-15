
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
		vec3 points[4];
		u32 p_size;

	public:
		Simplex()
      {
         points[0] = vec3(0);
         points[1] = vec3(0);
         points[2] = vec3(0);
         points[3] = vec3(0);
         p_size = 0;
      }

		Simplex& operator=(std::initializer_list<vec3> list) {
			for (auto v = list.begin(); v != list.end(); v++) {
				points[std::distance(list.begin(), v)] = *v;
			}
			p_size = list.size();

			return *this;
		}

		void push_front(vec3 point) {
         points[1] = points[0];
         points[2] = points[1];
         points[3] = points[2];
         points[0] = point;
         
			p_size = p_size + 1;
         assert(p_size <= 4);
		}

		vec3& operator[](u32 i) { return points[i]; }
		u32 size() const { return p_size; }
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

GJK_Point CL_find_furthest_vertex(Mesh* collision_mesh, vec3 direction)
{
	// linearly scan the CollisionMesh doing dot products with the vertices and storing the one with max value, then return it

   float max_inner_p = MIN_FLOAT;
   vec3 furthest_vertex;

   for (int i = 0; i < collision_mesh->vertices.size(); i++)
   {
      vec3 vertex_pos = collision_mesh->vertices[i].position;
      float inner_p = glm::dot(vertex_pos, direction);
      if(inner_p > max_inner_p)
      {
         max_inner_p = inner_p;
         furthest_vertex = vertex_pos;
      }
   }

   return GJK_Point{furthest_vertex, max_inner_p == MIN_FLOAT};
}


GJK_Point CL_get_support_point(Mesh* collision_mesh_A, Mesh* collision_mesh_B, vec3 direction)
{
   // Gets a support point in the minkowski difference of both meshes, in the direction supplied.

   GJK_Point gjk_point_a = CL_find_furthest_vertex(collision_mesh_A, direction);
   GJK_Point gjk_point_b = CL_find_furthest_vertex(collision_mesh_B, -1.f * direction);

   if (gjk_point_a.empty || gjk_point_b.empty)
      return gjk_point_a;

   return GJK_Point{gjk_point_a.point - gjk_point_b.point, false};
}

// -----------------------
// > UPDATE SIMPLEX CALLS
// -----------------------

GJK_Iteration CL_update_line_simplex(GJK_Iteration gjk)
{
   vec3 a = gjk.simplex[0];
   vec3 b = gjk.simplex[1];

   vec3 ab = b - a;
   vec3 ao =   - a;

   GJK_Iteration next;

   if (CL_same_general_direction(ab, ao))
   {
      next.simplex   = gjk.simplex;
      next.direction = cross(ab, ao, ab);
   }

   else
   {
      next.simplex   = { a };
      next.direction = ao;
   }

   return next;
}


GJK_Iteration CL_update_triangle_simplex(GJK_Iteration gjk)
{
   vec3 a = gjk.simplex[0];
   vec3 b = gjk.simplex[1];
   vec3 c = gjk.simplex[2];

   vec3 ab = b - a;
   vec3 ac = c - a;
   vec3 ao =   - a;
   
   vec3 abc = glm::cross(ab, ac);

   GJK_Iteration next;

   if(CL_same_general_direction(glm::cross(abc, ac), ao))
   {
      if(CL_same_general_direction(ac, ao))
      {
         next.simplex   = { a, c };
         next.direction = cross(ac, ao, ac);
      }
      else
      {
         next.simplex   = { a, b };
         next.direction = gjk.direction;
      }
   }

   else
   {
      if(CL_same_general_direction(glm::cross(ab, abc), ao))
      {
         next.simplex   = {a, b};
         next.direction = gjk.direction;
      }

      else
      {
         if(CL_same_general_direction(abc, ao))
         {
            next.simplex   = gjk.simplex;
            next.direction = abc;
         }
         else
         {
            next.simplex   = { a, c, b };
            next.direction = -1.f * abc;   
         }
      }
   }

   return next;
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

   GJK_Iteration next;

   // check if mikowski's diff origin is pointing towards tetrahedron normal faces
   // (it shouldn't, since the origin should be contained in the shape and point down not up like the inclined faces's normals)

   if(CL_same_general_direction(abc, ao))
   {
      next.simplex   = { a, b, c };
      next.direction = gjk.direction; 
      return CL_update_triangle_simplex(next);
   }

   if(CL_same_general_direction(acd, ao))
   {
      next.simplex   = { a, c, d };
      next.direction = gjk.direction; 
      return CL_update_triangle_simplex(next);
   }

   if(CL_same_general_direction(adb, ao))
   {
      next.simplex   = { a, d, b };
      next.direction = gjk.direction; 
      return CL_update_triangle_simplex(next);
   }

   // was not the case, found collision
   next.simplex = gjk.simplex;
   next.finished = true;
   return next;
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
   return gjk;
}


// ------------------
// > CL_RUN_GJK
// ------------------

bool CL_run_GJK(Entity entity_A, Entity entity_B)
{
   // Runs GJK algorithm
   Mesh* collider_A = &entity_A.collision_mesh;
   Mesh* collider_B = &entity_B.collision_mesh;

   GJK_Point support = CL_get_support_point(collider_A, collider_B, UNIT_X);

   if(support.empty)
      return false;

   GJK_Iteration gjk;
   gjk.simplex.push_front(support.point);
   gjk.direction = -1.0f * support.point;

   while(true)
   {
      support = CL_get_support_point(collider_A, collider_B, UNIT_X);

      if(support.empty)
         return false;

      if(!CL_same_general_direction(support.point, gjk.direction))
         return false;    // no collision

      gjk.simplex.push_front(support.point);

      gjk = CL_update_simplex_and_direction(gjk);

      if(gjk.finished)
         return true;
   }
}