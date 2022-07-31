#include <vector>
#include <string>
#include <map>
#include <engine/core/rvn_types.h>
#include <engine/collision/primitives/bounding_box.h>
#include <engine/collision/primitives/ray.h>
#include <engine/vertex.h>
#include <engine/mesh.h>
#include <colors.h>
#include <engine/render/renderer.h>
#include <engine/render/im_render.h>
#include <engine/collision/simplex.h>
#include <engine/collision/cl_gjk.h>
#include <chrono>
#include <iostream>

vec3 Debug_Colors[] = {
   COLOR_RED_1,
   COLOR_GREEN_1,
   COLOR_BLUE_1,
   COLOR_PURPLE_1,
};

/* -----------------------
  GJK Support Functions
----------------------- */
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

/* -----------------------
  Update simplex calls
----------------------- */
void CL_update_line_simplex(GJK_Iteration* gjk)
{
   auto& a = gjk->simplex[0];
   auto& b = gjk->simplex[1];

   vec3 ab = b - a;
   vec3 ao =   - a;

   if(CL_same_general_direction(ab, ao))
   {
      gjk->direction = cross(ab, ao, ab);
   }
   else
   {
      gjk->simplex   = { a };
      gjk->direction = ao;
   }
}


void CL_update_triangle_simplex(GJK_Iteration* gjk)
{
   auto& a = gjk->simplex[0];
   auto& b = gjk->simplex[1];
   auto& c = gjk->simplex[2];

   vec3 ab = b - a;
   vec3 ac = c - a;
   vec3 ao =   - a;
   
   vec3 abc = glm::cross(ab, ac);

   if(CL_same_general_direction(glm::cross(abc, ac), ao))
   {
      if(CL_same_general_direction(ac, ao))
      {
         // search in purple region
         gjk->simplex   = { a, c };
         gjk->direction = cross(ac, ao, ac);
      }
      else
      {
         // search in navy/blue grey region
         gjk->simplex   = { a, b };
         gjk->direction = -ac + (-ab);
      }
   }
   else
   {
      if(CL_same_general_direction(glm::cross(ab, abc), ao))
      {
         // search in cyan region
         gjk->simplex   = {a, b};
         gjk->direction = glm::cross(ab, abc);
      }
      else
      {
         if(CL_same_general_direction(abc, ao))
         {
            // search up
            gjk->direction = abc;
         }
         else
         {
            // search down
            gjk->simplex   = { a, c, b };
            gjk->direction = -abc;   
         }
      }
   }
}


void CL_update_tetrahedron_simplex(GJK_Iteration* gjk)
{
   auto& a = gjk->simplex[0];
	auto& b = gjk->simplex[1];
	auto& c = gjk->simplex[2];
	auto& d = gjk->simplex[3];

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
      gjk->simplex   = { a, b, c };
      gjk->direction = abc; 
      return CL_update_triangle_simplex(gjk);
   }

   if(CL_same_general_direction(acd, ao))
   {
      gjk->simplex   = { a, c, d };
      gjk->direction = acd; 
      return CL_update_triangle_simplex(gjk);
   }

   if(CL_same_general_direction(adb, ao))
   {
      gjk->simplex   = { a, d, b };
      gjk->direction = adb; 
      return CL_update_triangle_simplex(gjk);
   }

   // was not the case, found collision
   gjk->finished = true;
   return;
}


void CL_update_simplex_and_direction(GJK_Iteration* gjk)
{
	switch(gjk->simplex.size())
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


/* ------------------
   Run GJK
------------------ */
static void get_time(int elapsed)
{
   static std::vector<int> times;
   const int N = 100;

   times.push_back(elapsed);


   if(times.size() == N)
   {
      int sum = 0;
      for(int i = 0; i < times.size(); i++)
         sum += times[i];
      
      float average = sum * 1.0 / N;

      std::cout << "Average time spent on GJK: " << average << "\n"; 

      times.clear();
   }
}


void _CL_debug_render_simplex(Simplex simplex)
{
   for(int i = 0; i < simplex.size(); i++)
      ImDraw::add_point(IM_ITERHASH(i), simplex[i], 2.0, true, Debug_Colors[i]);
}


GJK_Result CL_run_GJK(Mesh* collider_A, Mesh* collider_B)
{
   using micro = std::chrono::microseconds;
   // auto start = std::chrono::high_resolution_clock::now(); 

   GJK_Point support = CL_get_support_point(collider_A, collider_B, UNIT_X);

   if(support.empty)
      return {};

   GJK_Iteration gjk;
   gjk.simplex.push_front(support.point);
   gjk.direction = -support.point;

   // ImDraw::add_point(IMHASH, support.point, 2.0, true, Debug_Colors[0]);

   int it_count = 0;
   while(true)
   {
      // auto start_it = std::chrono::high_resolution_clock::now(); 

      // auto start = std::chrono::high_resolution_clock::now(); 
      support = CL_get_support_point(collider_A, collider_B, gjk.direction);
      // auto finish = std::chrono::high_resolution_clock::now();

      // std::cout << "CL_get_support_point() took "
      //          << std::chrono::duration_cast<micro>(finish - start).count()
      //          << " microseconds\n";

      if(support.empty || !CL_same_general_direction(support.point, gjk.direction))
      {
         // _CL_debug_render_simplex(gjk.simplex);
         return {};    // no collision
      }

      gjk.simplex.push_front(support.point);

      // ImDraw::add_point(IM_ITERHASH(it_count), support.point, 2.0, true, Debug_Colors[it_count + 1]);

      CL_update_simplex_and_direction(&gjk);

      // auto finish_it = std::chrono::high_resolution_clock::now();

      // std::cout << "GJK Iteration took "
      //          << std::chrono::duration_cast<micro>(finish_it - start_it).count()
      //          << " microseconds\n";

      it_count++;
      if(gjk.finished)
      {
         // auto finish = std::chrono::high_resolution_clock::now();
         // int elapsed = std::chrono::duration_cast<micro>(finish - start).count();
         // get_time(elapsed);

         //_CL_debug_render_simplex(gjk.simplex);
         return {.simplex = gjk.simplex, .collision = true};
      }
   }
}