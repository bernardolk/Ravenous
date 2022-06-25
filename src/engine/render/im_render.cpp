#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <engine/core/rvn_types.h>
#include <rvn_macros.h>
#include <colors.h>
#include <engine/shader.h>
#include <engine/vertex.h>
#include <engine/mesh.h>
#include <glad/glad.h>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/triangle.h>
#include <engine/collision/primitives/bounding_box.h>
#include <glm/gtx/quaternion.hpp>
#include <engine/entity.h>
#include <engine/render/renderer.h>
#include <engine/camera.h>
#include <engine/render/im_render.h>

extern ImDraw IM_RENDER;

void ImDraw::init()
{
   this->list = new ImmediateDrawElement[IM_BUFFER_SIZE];
   for(int i = 0; i < IM_BUFFER_SIZE; i++)
   {
      _empty_slot(i);
      this->list[i].mesh.setup_gl_buffers();
   }
}

void ImDraw::update(float frame_duration)
{
   for (int i = 0; i < IM_BUFFER_SIZE; i++)
   {
      auto& obj = list[i];
      if(obj.empty)
         continue;
         
      obj.duration -= frame_duration * 1000.0;
      if(obj.duration <= 0)
         this->_empty_slot(i);
   }
}

void ImDraw::render(Camera* camera)
{
   Shader* im_point_shader    = Shader_Catalogue.find("immediate_point")->second;
   Shader* im_mesh_shader     = Shader_Catalogue.find("im_mesh")->second;
   Shader* shader             = im_point_shader;
   for(int i = 0; i < IM_BUFFER_SIZE; i++)
   {
      auto& obj = this->list[i];
      if(obj.empty) continue;

      vec3 color = obj.render_options.color.x == -1 ? vec3(0.9, 0.2, 0.0) : obj.render_options.color;

      if(obj.is_mesh)
      {
         shader = im_mesh_shader;
         shader->use();
         if(!obj.is_multpl_by_matmodel)
         {
            auto matModel = _get_mat_model(obj.pos, obj.rot, obj.scale);
            shader->setMatrix4("model", matModel);
         }
         else
         {
            shader->setMatrix4("model", mat4identity);
         }
      }
      else
      {
         shader = im_point_shader;
         shader->use();
      }

      shader->setMatrix4("view"        ,camera->View4x4);
      shader->setMatrix4("projection"  ,camera->Projection4x4);
      shader->setFloat  ("opacity"     ,obj.render_options.opacity);
      shader->setFloat3 ("color"       ,obj.render_options.color);

      render_mesh(&(list[i].mesh), obj.render_options);
   }
}

/* --------------------------- */
/*     > Private functions     */
/* --------------------------- */
void ImDraw::_empty_slot(int i)
{
   auto& obj = this->list[i];
   obj.mesh.indices.clear();
   obj.mesh.vertices.clear();
   obj.empty                    = true;
   obj.hash                     = 0;
   obj.duration                 = 0;
   obj.is_mesh                  = false;
   obj.scale                    = vec3(0);
   obj.pos                      = vec3(0);
   obj.rot                      = vec3(0);
   obj.is_multpl_by_matmodel    = false;
}


ImmediateDrawElementSlot ImDraw::_find_element_or_empty_slot(size_t hash)
{
   int slot = -1;
   for(int i = 0; i < IM_BUFFER_SIZE; i++)
   { 
      if(slot == -1 && this->list[i].empty)
         slot = i;
      if(this->list[i].hash == hash)
         return ImmediateDrawElementSlot { false, i };
   }

   if(slot == -1)
      std::cout << "IM RENDER BUFFER IS FULL\n";

   return ImmediateDrawElementSlot { true, slot };
}


void ImDraw::_set_mesh(int i, std::vector<Vertex> vertices, GLenum draw_method, RenderOptions opts)
{
   auto& obj = this->list[i];
   obj.mesh.vertices          = vertices;
   obj.mesh.render_method     = draw_method;
   obj.render_options         = opts;
   obj.empty                  = false;

   obj.mesh.send_data_to_gl_buffer();
}


void ImDraw::_set_mesh(int i, Mesh* mesh, RenderOptions opts)
{
   auto& obj = this->list[i];
   obj.mesh             = *mesh;
   obj.render_options   = opts;
   obj.is_mesh          = true;
   obj.empty            = false;
   obj.mesh.send_data_to_gl_buffer();
}


void ImDraw::_update_mesh(int i, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration)
{
   auto& obj = this->list[i];
   obj.render_options.color   = color;
   obj.duration               = duration;
   obj.pos                    = pos;
   obj.rot                    = rot;
   obj.scale                  = scale;
}


void ImDraw::_update_mesh(int i, vec3 color, int duration)
{
   auto& obj = this->list[i];
   obj.render_options.color   = color;
   obj.duration               = duration;
}


//@TODO: Probably redundant code
mat4 ImDraw::_get_mat_model(vec3 pos, vec3 rot, vec3 scale)
{
   glm::mat4 model = translate(mat4identity, pos);
   model = rotate(model, glm::radians(rot.x), vec3(1.0f, 0.0f, 0.0f));
   model = rotate(model, glm::radians(rot.y), vec3(0.0f, 1.0f, 0.0f));
   model = rotate(model, glm::radians(rot.z), vec3(0.0f, 0.0f, 1.0f));
   model = glm::scale(model, scale);
   return model;
}


void ImDraw::_set_indices(int i, std::vector<u32> indices)
{
   this->list[i].mesh.indices = indices;
}


/* --------------------------- */
/*      > Add primitives       */
/* --------------------------- */
void ImDraw::add(size_t _hash, std::vector<Vertex> vertex_vec, GLenum draw_method, RenderOptions opts)
{
   IM_R_FIND_SLOT();
   
   this->_set_mesh(slot.index, vertex_vec, draw_method, opts);
}


void ImDraw::add(size_t _hash, std::vector<Triangle> triangles, GLenum draw_method = GL_LINE_LOOP, RenderOptions opts = RenderOptions{})
{
   IM_R_FIND_SLOT();

   std::vector<Vertex> vertex_vec;
   for(int i = 0; i < triangles.size(); i++)
   {
      vertex_vec.push_back(Vertex{triangles[i].a});
      vertex_vec.push_back(Vertex{triangles[i].b});
      vertex_vec.push_back(Vertex{triangles[i].c});
   }
   
   this->_set_mesh(slot.index, vertex_vec, draw_method, opts);
}

void ImDraw::add_line(size_t _hash, vec3 pointA, vec3 pointB, vec3 color)
{
   this->add_line(_hash, pointA, pointB, 1.0, false, color);
}


void ImDraw::add_line(size_t _hash, vec3 pointA, vec3 pointB, float line_width, 
   bool always_on_top, vec3 color, float duration)
{
   IM_R_FIND_SLOT();

   auto vertex_vec = std::vector<Vertex>{ Vertex{pointA}, Vertex{pointB} };

   RenderOptions opts;
   opts.line_width      = line_width;
   opts.always_on_top   = always_on_top;
   opts.color           = color;
   opts.dont_cull_face  = true;

   if(duration != 0 )
   {
      auto& obj      = this->list[slot.index];
      obj.hash       = _hash;
      obj.duration   = duration;
   }

   this->_set_mesh(slot.index, vertex_vec, GL_LINES, opts);
}


void ImDraw::add_line_loop(size_t _hash, std::vector<vec3> points, float line_width, bool always_on_top)
{
   IM_R_FIND_SLOT();

   auto vertex_vec = std::vector<Vertex>();
   for(int i = 0; i < points.size(); i++)
      vertex_vec.push_back(Vertex{points[i]});

   RenderOptions opts;
   opts.line_width      = line_width;
   opts.always_on_top   = always_on_top;
   opts.dont_cull_face  = true;

   this->_set_mesh(slot.index, vertex_vec, GL_LINE_LOOP, opts);
}


void ImDraw::add_point(size_t _hash, vec3 point, float point_size, bool always_on_top, vec3 color, float duration)
{
   IM_R_FIND_SLOT();

   if(duration != 0 )
   {
      auto& obj      = this->list[slot.index];
      obj.hash       = _hash;
      obj.duration   = duration;
   }

   auto vertex_vec = std::vector<Vertex>{ Vertex{point} };

   RenderOptions opts;
   opts.point_size      = point_size;
   opts.always_on_top   = always_on_top;
   opts.color           = color;

   this->_set_mesh(slot.index, vertex_vec, GL_POINTS, opts);
}

void ImDraw::add_point(size_t _hash, vec3 point, vec3 color)
{
   this->add_point(_hash, point, 1.0, false, color);
}


void ImDraw::add_triangle(size_t _hash, Triangle t, float line_width, bool always_on_top, vec3 color)
{
   IM_R_FIND_SLOT();

   auto vertex_vec   = std::vector<Vertex>{ Vertex{t.a}, Vertex{t.b}, Vertex{t.c} };
   auto indices      = std::vector<u32>{ 0, 1, 2 };

   RenderOptions opts;
   opts.line_width      = line_width;
   opts.always_on_top   = always_on_top;
   opts.color           = color;

   this->_set_mesh(slot.index, vertex_vec, GL_TRIANGLES, opts);
   this->_set_indices(slot.index, indices);
}


/* --------------------------- */
/*        > Add Mesh           */
/* --------------------------- */
void ImDraw::add_mesh(size_t _hash, Mesh* mesh, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration)
{
   IM_R_FIND_SLOT();

   if(!slot.empty)
   {
      this->_update_mesh(slot.index, pos, rot, scale, color, duration);
      return;
   }

   RenderOptions opts;
   opts.color        = color;
   opts.wireframe    = true;

   auto& obj      = list[slot.index];
   obj.hash       = _hash;
   obj.duration   = duration;
   obj.pos        = pos;
   obj.rot        = rot;
   obj.scale      = scale;

   this->_set_mesh(slot.index, mesh, opts);
}


void ImDraw::add_mesh(size_t _hash, Mesh* mesh, vec3 color, float duration)
{
   IM_R_FIND_SLOT();

   if(!slot.empty)
   {
      this->_update_mesh(slot.index, color, duration);
      return;
   }

   RenderOptions opts;
   opts.color     = color;
   opts.wireframe = true;

   auto& obj                     = list[slot.index];
   obj.hash                      = _hash;
   obj.duration                  = duration;
   obj.is_multpl_by_matmodel     = true;

   this->_set_mesh(slot.index, mesh, opts);
}


void ImDraw::add_mesh(size_t _hash, Entity* entity, int duration)
{
   this->add_mesh(_hash, entity->mesh, entity->position, entity->rotation, entity->scale, vec3(1.0,0,0), duration);
}


void ImDraw::add_mesh(size_t _hash, Entity* entity)
{
   this->add_mesh(_hash, entity->mesh, entity->position, entity->rotation, entity->scale);
}


void ImDraw::add_mesh(size_t _hash, Entity* entity, vec3 pos)
{
   this->add_mesh(_hash, entity->mesh, pos, entity->rotation, entity->scale);
}