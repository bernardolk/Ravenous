// -----------------------
// GLOBAL IMMEDIATE DRAW 
// -----------------------
// This module allows for adding geometric primitives to a buffer
// and render them each frame from anywhere in the code, mostly
// for debugging purposes

struct ImmediateDrawElement {
   bool empty;
   Mesh mesh;
   RenderOptions render_options;
   i16 duration;
};

struct GlobalImmediateDraw {
   const static int IM_BUFFER_SIZE = 100;
   ImmediateDrawElement* list;

   void init()
   {
      list = new ImmediateDrawElement[IM_BUFFER_SIZE];
      for(int i = 0; i < IM_BUFFER_SIZE; i++)
      {
         auto obj = &list[i];
         obj->mesh.setup_gl_buffers();
         obj->duration = 0;
         obj->empty = true;
      }
   }

   int _find_space()
   {
      for(int i = 0; i < IM_BUFFER_SIZE; i++)
      { 
         if(list[i].empty)
            return i;
      }

      cout << "IM RENDER BUFFER IS FULL\n";
      return -1;
   }

   void _set_mesh(int i, vector<Vertex> vertices, GLenum draw_method, RenderOptions opts)
   {
      auto obj = &list[i];
      obj->mesh.vertices = vertices;
      obj->mesh.render_method = draw_method;
      obj->mesh.send_data_to_gl_buffer();
      obj->render_options = opts;
      obj->empty = false;
   }

   void _set_indices(int i, vector<u32> indices)
   {
      list[i].mesh.indices = indices;
   }

   void add(vector<Vertex> vertex_vec, GLenum draw_method, RenderOptions opts = RenderOptions{})
   {
      int i = _find_space();
      if(i == -1) return;
      
      _set_mesh(i, vertex_vec, draw_method, opts);
   }

   void add(vector<Triangle> triangles, GLenum draw_method = GL_LINE_LOOP, RenderOptions opts = RenderOptions{})
   {
      int i = _find_space();
      if(i == -1) return;

      vector<Vertex> vertex_vec;
      for(int i = 0; i < triangles.size(); i++)
      {
         vertex_vec.push_back(Vertex{triangles[i].a});
         vertex_vec.push_back(Vertex{triangles[i].b});
         vertex_vec.push_back(Vertex{triangles[i].c});
      }
      
      _set_mesh(i, vertex_vec, draw_method, opts);
   }

   void add_line(vec3 points[2], float line_width = 1.0, bool always_on_top = false)
   {
      int i = _find_space();
      if(i == -1) return;

      auto vertex_vec = vector<Vertex>{ Vertex{points[0]}, Vertex{points[1]} };

      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;

      _set_mesh(i, vertex_vec, GL_LINES, opts);
   }

   void add_line(vec3 pointA, vec3 pointB, float line_width = 1.0, bool always_on_top = false)
   {
      int i = _find_space();
      if(i == -1) return;

      auto vertex_vec = vector<Vertex>{ Vertex{pointA}, Vertex{pointB} };

      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;

      _set_mesh(i, vertex_vec, GL_LINES, opts);
   }

   void add_line_loop(vector<vec3> points, float line_width = 1.0, bool always_on_top = false)
   {
      int i = _find_space();
      if(i == -1) return;

      auto vertex_vec = vector<Vertex>();
      for(int i = 0; i < points.size(); i++)
         vertex_vec.push_back(Vertex{points[i]});

      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;

      _set_mesh(i, vertex_vec, GL_LINE_LOOP, opts);
   }

   void add_point(vec3 point, float point_size = 1.0, bool always_on_top = false)
   {
      int i = _find_space();
      if(i == -1) return;

      auto vertex_vec = vector<Vertex>{ Vertex{point} };

      RenderOptions opts;
      opts.point_size = point_size;
      opts.always_on_top = always_on_top;

      _set_mesh(i, vertex_vec, GL_POINTS, opts);
   }

   void add_triangle(Triangle t, float line_width = 1.0, bool always_on_top = false, vec3 color = vec3{0.8, 0.2, 0.2})
   {
      int i = _find_space();
      if(i == -1) return;

      auto vertex_vec = vector<Vertex>{ Vertex{t.a}, Vertex{t.b}, Vertex{t.c} };
      auto indices = vector<u32>{ 0, 1, 2 };

      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;
      opts.color = color;

      _set_mesh(i, vertex_vec, GL_TRIANGLES, opts);
      _set_indices(i, indices);
   }

   void check_expired_entries()
   {
      for (int i = 0; i < IM_BUFFER_SIZE; i++)
      {
         auto obj = &list[i];
         obj->duration -= G_FRAME_INFO.duration * 1000.0;
         if(obj->duration <= 0)
         {
            obj->mesh.indices.clear();
            obj->mesh.vertices.clear();
            obj->empty = true;
            obj->duration = 0;
         }
      }
   }

   void render()
   {
      auto shader = Shader_Catalogue.find("immediate_point")->second;
      for(int i = 0; i < IM_BUFFER_SIZE; i++)
      {
         auto obj = &list[i];
         if(obj->empty) continue;

         vec3 color = obj->render_options.color.x == -1 ? vec3(0.9, 0.2, 0.0) : obj->render_options.color;

         shader->use();
         shader->setMatrix4("view"        ,G_SCENE_INFO.camera->View4x4);
         shader->setMatrix4("projection"  ,G_SCENE_INFO.camera->Projection4x4);
         shader->setFloat  ("opacity"     ,obj->render_options.opacity);
         shader->setFloat3 ("color"       ,obj->render_options.color);

         render_mesh(&(list[i].mesh), obj->render_options);
      }
   }

} G_IMMEDIATE_DRAW;
