#include <engine/core/rvn_types.h>
#include <vector>
#include <string>
#include <iostream>
#include <engine/vertex.h>
#include <map>
#include <engine/mesh.h>
#include <glad/glad.h>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/triangle.h>
#include <engine/render/renderer.h>

void render_mesh(Mesh* mesh, RenderOptions opts)
{
   glBindVertexArray(mesh->gl_data.VAO);

   // set render modifiers
   if(opts.wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   if(opts.always_on_top)
      glDepthFunc(GL_ALWAYS);
   if(opts.point_size != 1.0)
      glPointSize(opts.point_size);
   if(opts.line_width != 1.0)
      glLineWidth(opts.line_width);
   if(opts.dont_cull_face)
      glDisable(GL_CULL_FACE);

   // draw
   switch (mesh->render_method)
   {
      case GL_TRIANGLE_STRIP:
         glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh->vertices.size());
         break;
      case GL_LINE_LOOP:
         glDrawArrays(GL_LINE_LOOP, 0, mesh->vertices.size());
         break;
      case GL_POINTS:
         glDrawArrays(GL_POINTS, 0, mesh->vertices.size());
         break;
      case GL_LINES:
         glDrawArrays(GL_LINES, 0, mesh->vertices.size());
         break;
      case GL_TRIANGLES:
         glDrawElements(GL_TRIANGLES,  mesh->indices.size(), GL_UNSIGNED_INT, 0);
         //glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());
         break;
      default:
         std::cout << "WARNING: no drawing method set for mesh '" << mesh->name << "', "<< 
            "it won't be rendered!\n";
   }

   // set to defaults
   if(opts.wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   if(opts.always_on_top)
      glDepthFunc(GL_LESS);
   if(opts.point_size != 1.0)
      glPointSize(1.0);
   if(opts.line_width != 1.0)
      glLineWidth(1.0);
   if(opts.dont_cull_face)
      glEnable(GL_CULL_FACE);

   glBindVertexArray(0);
}