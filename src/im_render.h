// -----------------------
// GLOBAL IMMEDIATE DRAW 
// -----------------------

/* --------------------------- */
/*      > Instructions         */
/* --------------------------- */
/* This module allows for adding geometric primitives to a buffer and render them each frame from anywhere in the code, mostly
 for debugging purposes. To use, simply add IM_RENDER.<function>(IMHASH, <args>) to your code. The IMHASH macro will expand to
 a hash calculation based on file and line of the function call. This way we can 'keep alive' the obj in the buffer if it is
 being requested to be updated, instead of clearing it and reseting it.
*/


/* --------------------------- */
/*           > Macros          */
/* --------------------------- */
// use IMCUSTOMHASH when you need to run IM_RENDER in a loop. In that case, __FILE__ and __LINE__ doesn't cut it,
// as in every iteration the item being added would be replaced because it would have the same hash always.
// Put a prefix + i from the loop in it and you should be fine.

#define IMCUSTOMHASH(x) im_hasher(x)
#define IM_ITERHASH(x) im_hasher(std::string(__FILE__) + "-" + std::to_string(__LINE__) + "-" + std::to_string(x))
#define IMHASH im_hasher(std::string(__FILE__) + "-" + std::to_string(__LINE__))
#define IM_R_FIND_SLOT() ImmediateDrawElementSlot slot = _find_element_or_empty_slot(_hash); \
                         if(slot.empty && slot.index == -1) return;

std::hash<std::string> im_hasher;

struct ImmediateDrawElement {
   size_t hash;
   bool empty;
   Mesh mesh;
   RenderOptions render_options;
   int duration;
   bool is_mesh;
   vec3 pos;
   vec3 rot;
   vec3 scale;
   bool is_multpl_by_matmodel;
};

struct ImmediateDrawElementSlot {
   bool empty;
   int index;
};

struct GlobalImmediateDraw {
   const static int IM_BUFFER_SIZE = 200;
   ImmediateDrawElement* list;

   void init()
   {
      list = new ImmediateDrawElement[IM_BUFFER_SIZE];
      for(int i = 0; i < IM_BUFFER_SIZE; i++)
      {
         _empty_slot(i);
         (&list[i])->mesh.setup_gl_buffers();
      }
   }

   /* --------------------------- */
   /*     > Private functions     */
   /* --------------------------- */
   void _empty_slot(int i)
   {
      auto obj = &list[i];
      obj->mesh.indices.clear();
      obj->mesh.vertices.clear();
      obj->empty                    = true;
      obj->hash                     = 0;
      obj->duration                 = 0;
      obj->is_mesh                  = false;
      obj->scale                    = vec3(0);
      obj->pos                      = vec3(0);
      obj->rot                      = vec3(0);
      obj->is_multpl_by_matmodel    = false;
   }

   ImmediateDrawElementSlot _find_element_or_empty_slot(size_t hash)
   {
      int slot = -1;
      for(int i = 0; i < IM_BUFFER_SIZE; i++)
      { 
         if(slot == -1 && list[i].empty)
            slot = i;
         if(list[i].hash == hash)
            return ImmediateDrawElementSlot { false, i };
      }

      if(slot == -1)
         std::cout << "IM RENDER BUFFER IS FULL\n";

      return ImmediateDrawElementSlot { true, slot };
   }

   void _set_mesh(int i, std::vector<Vertex> vertices, GLenum draw_method, RenderOptions opts)
   {
      auto obj = &list[i];
      obj->mesh.vertices = vertices;
      obj->mesh.render_method = draw_method;
      obj->mesh.send_data_to_gl_buffer();
      obj->render_options = opts;
      obj->empty = false;
      // missing is mesh?
   }

   void _set_mesh(int i, Mesh* mesh, RenderOptions opts)
   {
      auto obj = &list[i];
      obj->mesh = *mesh;
      obj->mesh.send_data_to_gl_buffer();
      obj->render_options = opts;
      obj->is_mesh = true;
      obj->empty = false;
   }

   void _update_mesh(int i, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration)
   {
      auto obj = &list[i];
      obj->render_options.color = color;
      obj->duration  = duration;
      obj->pos       = pos;
      obj->rot       = rot;
      obj->scale     = scale;
   }

   void _update_mesh(int i, vec3 color, int duration)
   {
      auto obj = &list[i];
      obj->render_options.color = color;
      obj->duration  = duration;
   }


   mat4 _get_mat_model(vec3 pos, vec3 rot, vec3 scale)
   {
      glm::mat4 model = translate(mat4identity, pos);
		model = rotate(model, glm::radians(rot.x), vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, glm::radians(rot.y), vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, glm::radians(rot.z), vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, scale);
		return model;
   }

   void _set_indices(int i, std::vector<u32> indices)
   {
      list[i].mesh.indices = indices;
   }

   /* --------------------------- */
   /*      > Add primitives       */
   /* --------------------------- */
   void add(size_t _hash, std::vector<Vertex> vertex_vec, GLenum draw_method, RenderOptions opts = RenderOptions{})
   {
      IM_R_FIND_SLOT();
      
      _set_mesh(slot.index, vertex_vec, draw_method, opts);
   }

   void add(size_t _hash, std::vector<Triangle> triangles, GLenum draw_method = GL_LINE_LOOP, RenderOptions opts = RenderOptions{})
   {
      IM_R_FIND_SLOT();

      std::vector<Vertex> vertex_vec;
      for(int i = 0; i < triangles.size(); i++)
      {
         vertex_vec.push_back(Vertex{triangles[i].a});
         vertex_vec.push_back(Vertex{triangles[i].b});
         vertex_vec.push_back(Vertex{triangles[i].c});
      }
      
      _set_mesh(slot.index, vertex_vec, draw_method, opts);
   }

   void add_line(size_t _hash, vec3 pointA, vec3 pointB, vec3 color)
   {
      add_line(_hash, pointA, pointB, 1.0, false, color);
   }
   
   void add_line(size_t _hash, vec3 pointA, vec3 pointB, float line_width = 1.0, 
      bool always_on_top = false, vec3 color = vec3(0), float duration = 0)
   {
      IM_R_FIND_SLOT();

      auto vertex_vec = std::vector<Vertex>{ Vertex{pointA}, Vertex{pointB} };

      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;
      opts.color = color;
      opts.dont_cull_face = true;

      if(duration != 0 )
      {
         auto obj = &list[slot.index];
         obj->hash = _hash;
         obj->duration = duration;
      }

      _set_mesh(slot.index, vertex_vec, GL_LINES, opts);
   }

   void add_line_loop(size_t _hash, std::vector<vec3> points, float line_width = 1.0, bool always_on_top = false)
   {
      IM_R_FIND_SLOT();

      auto vertex_vec = std::vector<Vertex>();
      for(int i = 0; i < points.size(); i++)
         vertex_vec.push_back(Vertex{points[i]});

      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;
      opts.dont_cull_face = true;

      _set_mesh(slot.index, vertex_vec, GL_LINE_LOOP, opts);
   }

   void add_point(size_t _hash, vec3 point, float point_size = 1.0, bool always_on_top = false, vec3 color = vec3(0), float duration = 0)
   {
      IM_R_FIND_SLOT();

      if(duration != 0 )
      {
         auto obj = &list[slot.index];
         obj->hash = _hash;
         obj->duration = duration;
      }

      auto vertex_vec = std::vector<Vertex>{ Vertex{point} };

      RenderOptions opts;
      opts.point_size = point_size;
      opts.always_on_top = always_on_top;
      opts.color = color;

      _set_mesh(slot.index, vertex_vec, GL_POINTS, opts);
   }

   void add_point(size_t _hash, vec3 point, vec3 color = vec3(0))
   {
      add_point(_hash, point, 1.0, false, color);
   }


   void add_triangle(size_t _hash, Triangle t, float line_width = 1.0, bool always_on_top = false, vec3 color = vec3{0.8, 0.2, 0.2})
   {
      IM_R_FIND_SLOT();

      auto vertex_vec = std::vector<Vertex>{ Vertex{t.a}, Vertex{t.b}, Vertex{t.c} };
      auto indices = std::vector<u32>{ 0, 1, 2 };

      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;
      opts.color = color;

      _set_mesh(slot.index, vertex_vec, GL_TRIANGLES, opts);
      _set_indices(slot.index, indices);
   }

   /* --------------------------- */
   /*        > Add Mesh           */
   /* --------------------------- */
   void add_mesh(size_t _hash, Mesh* mesh, vec3 pos, vec3 rot, vec3 scale, vec3 color = COLOR_BLUE_1, int duration = 2000)
   {
      IM_R_FIND_SLOT();
 
      if(!slot.empty)
      {
         _update_mesh(slot.index, pos, rot, scale, color, duration);
         return;
      }

      RenderOptions opts;
      opts.color = color;
      opts.wireframe = true;

      auto obj = &list[slot.index];
      obj->hash = _hash;
      obj->duration = duration;
      obj->pos = pos;
      obj->rot = rot;
      obj->scale = scale;

      _set_mesh(slot.index, mesh, opts);
   }

   void add_mesh(size_t _hash, Mesh* mesh, vec3 color = COLOR_BLUE_1, float duration = 2000)
   {
      IM_R_FIND_SLOT();
 
      if(!slot.empty)
      {
         _update_mesh(slot.index, color, duration);
         return;
      }

      RenderOptions opts;
      opts.color     = color;
      opts.wireframe = true;

      auto obj                      = &list[slot.index];
      obj->hash                     = _hash;
      obj->duration                 = duration;
      obj->is_multpl_by_matmodel    = true;

      _set_mesh(slot.index, mesh, opts);
   }


   void add_mesh(size_t _hash, Entity* entity, int duration)
   {
     add_mesh(_hash, entity->mesh, entity->position, entity->rotation, entity->scale, vec3(1.0,0,0), duration);
   }

   void add_mesh(size_t _hash, Entity* entity)
   {
     add_mesh(_hash, entity->mesh, entity->position, entity->rotation, entity->scale);
   }

   void add_mesh(size_t _hash, Entity* entity, vec3 pos)
   {
     add_mesh(_hash, entity->mesh, pos, entity->rotation, entity->scale);
   }


   /* --------------------------- */
   /*      > Functionalities      */
   /* --------------------------- */
   void check_expired_entries()
   {
      for (int i = 0; i < IM_BUFFER_SIZE; i++)
      {
         auto obj = &list[i];
         if(obj->empty)
            continue;
            
         obj->duration -= G_FRAME_INFO.duration * 1000.0;
         if(obj->duration <= 0)
            _empty_slot(i);
      }
   }

   void render()
   {
      Shader* im_point_shader = Shader_Catalogue.find("immediate_point")->second;
      Shader* im_mesh_shader = Shader_Catalogue.find("im_mesh")->second;
      Shader* shader = im_point_shader;
      for(int i = 0; i < IM_BUFFER_SIZE; i++)
      {
         auto obj = &list[i];
         if(obj->empty) continue;

         vec3 color = obj->render_options.color.x == -1 ? vec3(0.9, 0.2, 0.0) : obj->render_options.color;

         if(obj->is_mesh)
         {
            shader = im_mesh_shader;
            shader->use();
            if(!obj->is_multpl_by_matmodel)
            {
               auto matModel = _get_mat_model(obj->pos, obj->rot, obj->scale);
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

         shader->setMatrix4("view"        ,G_SCENE_INFO.camera->View4x4);
         shader->setMatrix4("projection"  ,G_SCENE_INFO.camera->Projection4x4);
         shader->setFloat  ("opacity"     ,obj->render_options.opacity);
         shader->setFloat3 ("color"       ,obj->render_options.color);

         render_mesh(&(list[i].mesh), obj->render_options);
      }
   }

} IM_RENDER;
