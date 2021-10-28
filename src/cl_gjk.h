
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
		vec3 points[4];
		u32 p_size;

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
         points[3] = points[2];
         points[2] = points[1];
         points[1] = points[0];
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
// > GJK_Result
// ------------------
struct GJK_Result {
   Simplex simplex;
   bool collision;
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
	// Linearly scan the CollisionMesh doing dot products with the vertices and storing the one with max value, then return it
   // Note: sometimes, the dot product between two points equals the same, but there is always a right and a wrong pair 
   // of support points that are ideal to go together. We are not storing these 'dispute points' and testing between them for
   // the sake of simplicity. If anything, it should take a bit more iteration to find a solution but it should work either
   // way.

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
   GJK_Point gjk_point_b = CL_find_furthest_vertex(collision_mesh_B, -direction);

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
         // search in purple region
         next.simplex   = { a, c };
         next.direction = cross(ac, ao, ac);
      }
      else
      {
         // search in navy/blue grey region
         next.simplex   = { a, b };
         next.direction = -ac + (-ab);
      }
   }

   else
   {
      if(CL_same_general_direction(glm::cross(ab, abc), ao))
      {
         // search in cyan region
         next.simplex   = {a, b};
         next.direction = glm::cross(ab, abc);
      }

      else
      {
         if(CL_same_general_direction(abc, ao))
         {
            // search up
            next.simplex   = gjk.simplex;
            next.direction = abc;
         }
         else
         {
            // search down
            next.simplex   = { a, c, b };
            next.direction = -abc;   
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
      next.direction = abc; 
      return CL_update_triangle_simplex(next);
   }

   if(CL_same_general_direction(acd, ao))
   {
      next.simplex   = { a, c, d };
      next.direction = acd; 
      return CL_update_triangle_simplex(next);
   }

   if(CL_same_general_direction(adb, ao))
   {
      next.simplex   = { a, d, b };
      next.direction = adb; 
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
vec3 Debug_Colors[] = {
   vec3(0.60, 0, 0),
   vec3(0.00, 0.60, 0),
   vec3(0.00, 0, 0.60),
   vec3(0.80, 0.80, 0.80),
};


void _CL_debug_render_simplex(Simplex simplex)
{
   for(int i = 0; i < simplex.size(); i++)
      IM_RENDER.add_point(IMHASH, simplex[i], 2.0, true, Debug_Colors[i]);
}


GJK_Result CL_run_GJK(Mesh* collider_A, Mesh* collider_B)
{
   GJK_Result result;
   result.collision = false;

   GJK_Point support = CL_get_support_point(collider_A, collider_B, UNIT_X);

   if(support.empty)
      return result;

   GJK_Iteration gjk;
   gjk.simplex.push_front(support.point);
   gjk.direction = -support.point;

   //IM_RENDER.add_point(IMHASH, support.point, 2.0, true, Debug_Colors[0]);

   int it_count = 0;
   while(true)
   {
      support = CL_get_support_point(collider_A, collider_B, gjk.direction);

      if(support.empty || !CL_same_general_direction(support.point, gjk.direction))
      {
         // _CL_debug_render_simplex(gjk.simplex);
         return result;    // no collision
      }

      gjk.simplex.push_front(support.point);

      //IM_RENDER.add_point(IMHASH, support.point, 2.0, true, Debug_Colors[it_count + 1]);

      gjk = CL_update_simplex_and_direction(gjk);

      it_count++;
      if(gjk.finished)
      {
         // _CL_debug_render_simplex(gjk.simplex);
         result.simplex = gjk.simplex;
         result.collision = true;
         return result;
      }
   }
}

