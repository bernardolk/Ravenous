// -------------------------
// RENDERING OPTIONS STRUCT
// -------------------------
struct RenderOptions
{
   bool wireframe = false;
   bool always_on_top = false;

   float point_size = 1.0;
   float line_width = 1.0;

   // for immediate point shader
   vec3 color = vec3{-1.0};
   float opacity = 1.0;
};

// -----------------------------
// GLOBAL IMMEDIATE DRAW STRUCT
// -----------------------------
struct GlobalImmediateDraw {
   const static int IM_BUFFER_SIZE = 20;
   Mesh* meshes[IM_BUFFER_SIZE];
   RenderOptions render_opts[IM_BUFFER_SIZE];
   int ind = 0;

   void init()
   {
      for(int i = 0; i < IM_BUFFER_SIZE; i++)
      {
         auto mesh = new Mesh();
         mesh->setup_gl_buffers();
         meshes[i] = mesh;
      }
   }

   void add(vector<Vertex> vertex_vec, GLenum draw_method, RenderOptions opts = RenderOptions{})
   {
      auto mesh = meshes[ind];
      mesh->vertices = vertex_vec;
      mesh->render_method = draw_method;
      mesh->send_data_to_gl_buffer();
      render_opts[ind] = opts;
      ind++;
   }

   void add(vector<Triangle> triangles, GLenum draw_method = GL_LINE_LOOP, RenderOptions opts = RenderOptions{})
   {
      vector<Vertex> vertices;
      for(int i = 0; i < triangles.size(); i++)
      {
         vertices.push_back(Vertex{triangles[i].a});
         vertices.push_back(Vertex{triangles[i].b});
         vertices.push_back(Vertex{triangles[i].c});
      }
      
      auto mesh = meshes[ind];
      mesh->vertices = vertices;
      mesh->render_method = draw_method;
      mesh->send_data_to_gl_buffer();
      ind++;
   }

   void add_line(vec3 points[2], float line_width = 1.0, bool always_on_top = false)
   {
      auto mesh = meshes[ind];
      mesh->vertices = vector<Vertex>{ Vertex{points[0]}, Vertex{points[1]}};
      mesh->render_method = GL_LINES;
      mesh->send_data_to_gl_buffer();
      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;
      render_opts[ind] = opts;
      ind++;
   }

   void add_lines(vector<vec3> points, float line_width = 1.0, bool always_on_top = false)
   {
      auto mesh = meshes[ind];
      mesh->vertices = vector<Vertex>();
      for(int i = 0; i < points.size(); i++)
      {
         mesh->vertices.push_back(Vertex{points[i]});
      }
      mesh->render_method = GL_LINE_LOOP;
      mesh->send_data_to_gl_buffer();
      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;
      render_opts[ind] = opts;
      ind++;
   }

   void add_point(vec3 point, float point_size = 1.0, bool always_on_top = false)
   {
      auto mesh = meshes[ind];
      mesh->vertices = vector<Vertex>{ Vertex{point} };
      mesh->render_method = GL_POINTS;
      mesh->send_data_to_gl_buffer();
      RenderOptions opts;
      opts.point_size = point_size;
      opts.always_on_top = always_on_top;
      render_opts[ind] = opts;
      ind++;
   }

   void add_triangle(Triangle t, float line_width = 1.0, bool always_on_top = false, vec3 color = vec3{0.8, 0.2, 0.2})
   {
      auto mesh = meshes[ind];
      mesh->vertices = vector<Vertex>{ Vertex{t.a}, Vertex{t.b}, Vertex{t.c}};
      mesh->indices = vector<unsigned int>{ 0, 1, 2};
      mesh->render_method = GL_TRIANGLES;
      mesh->send_data_to_gl_buffer();
      RenderOptions opts;
      opts.line_width = line_width;
      opts.always_on_top = always_on_top;
      opts.color = color;
      render_opts[ind] = opts;
      ind++;
   }

   void reset()
   {
      for (int i = 0; i < ind; i++)
      {
         auto mesh = meshes[i];
         mesh->indices.clear();
         mesh->vertices.clear();
      }
      ind = 0;
   }
} G_IMMEDIATE_DRAW;


void render_text(float x, float y, string text);
void render_text(float x, float y, vec3 color, string text);
void render_text(string font, float x, float y, string text);
void render_text(string font, float x, float y, bool center, string text);
void render_text(string font, float x, float y, vec3 color, string text);
void render_text(string font, float x, float y, vec3 color, bool center, string text);
void render_text(string font, float x, float y, float scale, string text);
void render_text(string font, float x, float y, vec3 color, float scale, string text);
void render_text(string font, float x, float y, vec3 color, float scale, bool center, string text);
void render_scene(Scene* scene, Camera* camera);
void render_entity(Entity* entity);
void render_editor_entity(Entity* entity, Scene* scene, Camera* camera);
void render_mesh(Mesh* mesh, RenderOptions opts = RenderOptions{});
void render_message_buffer_contents();
void render_immediate(GlobalImmediateDraw* im, Camera* camera);


// ------------
// RENDER MESH
// ------------
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
         break;
      default:
         cout << "WARNING: no drawing method set for mesh '" << mesh->name << "', "<< 
            "it won't be rendered!\n";
   }

   // set to normal
   if(opts.wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   if(opts.always_on_top)
      glDepthFunc(GL_LESS);
   if(opts.point_size != 1.0)
      glPointSize(1.0);
   if(opts.line_width != 1.0)
      glLineWidth(1.0);

   glBindVertexArray(0);
}

// --------------
// RENDER ENTITY
// --------------
void render_entity(Entity* entity)
{
   // bind appropriate textures
   unsigned int diffuseNr = 1;
   unsigned int specularNr = 1;
   unsigned int normalNr = 1;
   unsigned int heightNr = 1;
   for (unsigned int i = 0; i < entity->textures.size(); i++)
   {
      glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
      // retrieve texture number (the N in diffuse_textureN)
      string number;
      string name = entity->textures[i].type;
      if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
      else if (name == "texture_specular")
            number = std::to_string(specularNr++); // transfer unsigned int to stream
      else if (name == "texture_normal")
            number = std::to_string(normalNr++); // transfer unsigned int to stream
      else if (name == "texture_height")
            number = std::to_string(heightNr++); // transfer unsigned int to stream

      // now set the sampler to the correct texture unit
      glUniform1i(glGetUniformLocation(entity->shader->gl_programId, (name + number).c_str()), i);
      // and finally bind the texture
      glBindTexture(GL_TEXTURE_2D, entity->textures[i].id);
   }

   // draw mesh
   auto render_opts = RenderOptions{entity->wireframe};
   render_mesh(entity->mesh, render_opts);

   // always good practice to set everything back to defaults once configured.
   glActiveTexture(GL_TEXTURE0);
}

void render_editor_entity(Entity* entity, Scene* scene, Camera* camera)
{
   entity->shader->use();
   // important that the gizmo dont have a position set.
   entity->shader->setMatrix4("model", entity->matModel);
   entity->shader->setMatrix4("view", camera->View4x4);
   entity->shader->setMatrix4("projection", camera->Projection4x4);
   entity->shader->setFloat3("viewPos", camera->Position);
   entity->shader->setFloat("shininess", scene->global_shininess);
   entity->shader->setMatrix4("model", entity->matModel);

   render_entity(entity);
}

// -------------
// RENDER SCENE
// -------------
void render_scene(Scene* scene, Camera* camera) 
{
   // set shader settings that are common to the scene
   auto shader = Shader_Catalogue.find("model")->second;
   shader->use();

   int point_light_count = 0;
   for (auto point_light_ptr = scene->pointLights.begin(); 
      point_light_ptr != scene->pointLights.end(); 
      point_light_ptr++)
   {
      PointLight point_light = *point_light_ptr;
      string uniform_name = "pointLights[" + to_string(point_light_count) + "]";
      shader->setFloat3(uniform_name + ".position",  point_light.position);
      shader->setFloat3(uniform_name + ".diffuse",   point_light.diffuse);
      shader->setFloat3(uniform_name + ".specular",  point_light.specular);
      shader->setFloat(uniform_name  + ".constant",  point_light.intensity_constant);
      shader->setFloat(uniform_name  + ".linear",    point_light.intensity_linear);
      shader->setFloat(uniform_name  + ".quadratic", point_light.intensity_quadratic);
      point_light_count++;
   }

   int spotlight_count = 0;
   for (auto spotlight_ptr = scene->spotLights.begin(); 
   spotlight_ptr != scene->spotLights.end(); 
   spotlight_ptr++)
   {
      SpotLight spotlight = *spotlight_ptr;
      string uniform_name = "spotLights[" + to_string(spotlight_count) + "]";
      shader->setFloat3(uniform_name + ".position",  spotlight.position);
      shader->setFloat3(uniform_name + ".direction", spotlight.direction);
      shader->setFloat3(uniform_name + ".diffuse",   spotlight.diffuse);
      shader->setFloat3(uniform_name + ".specular",  spotlight.specular);
      shader->setFloat(uniform_name  + ".constant",  spotlight.intensity_constant);
      shader->setFloat(uniform_name  + ".linear",    spotlight.intensity_linear);
      shader->setFloat(uniform_name  + ".quadratic", spotlight.intensity_quadratic);
      shader->setFloat(uniform_name + ".innercone", spotlight.innercone);
      shader->setFloat(uniform_name + ".outercone", spotlight.outercone);
      spotlight_count++;
   }

   shader->     setInt("num_directional_light", 0);
   shader->     setInt("num_spot_lights",     spotlight_count);
   shader->     setInt("num_point_lights",    point_light_count);
   shader-> setMatrix4("view",                camera->View4x4);
   shader-> setMatrix4("projection",          camera->Projection4x4);
   shader->   setFloat("shininess",           scene->global_shininess);
   shader->  setFloat3("ambient",             scene->ambient_light);
   shader->   setFloat("ambient_intensity",   scene->ambient_intensity);
   shader->  setFloat3("viewPos",             camera->Position);

	Entity **entity_iterator = &(scene->entities[0]);
   int entities_vec_size =  scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;
      if(!entity->render_me)
         continue;

      entity->shader-> setMatrix4("model", entity->matModel);
      render_entity(entity);
	}
}

// -------------------------
// RENDER GAME GUI
// -------------------------
void render_game_gui(Player* player)
{
   auto color = player->lives == 2 ? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1};
   render_text("consola42", 25, 75, color, to_string(player->lives));
}

// -----------------------------
// RENDER IMMEDIATE DRAW BUFFER
// -----------------------------
void render_immediate(GlobalImmediateDraw* im, Camera* camera)
{
   auto shader = Shader_Catalogue.find("immediate_point")->second;
   for(int i = 0; i < im->ind; i++)
   {
      auto opts = im->render_opts[i];
      vec3 color = opts.color.x == -1 ? vec3(0.9, 0.2, 0.0) : opts.color;

      auto mesh = im->meshes[i];
      shader-> use();
      shader-> setMatrix4("view",        camera->View4x4);
      shader-> setMatrix4("projection",  camera->Projection4x4);
      shader-> setFloat("opacity", opts.opacity);
      shader-> setFloat3("color", color);
      render_mesh(mesh, opts);
   }
   
   G_IMMEDIATE_DRAW.reset();
}

// ------------
// RENDER TEXT
// ------------
void render_text(float x, float y, string text) 
{
   render_text("consola12", x, y, vec3{1.0, 1.0, 1.0}, 1.0, false, text);
}

void render_text(string font, float x, float y, string text) 
{
   render_text(font, x, y, vec3{1.0, 1.0, 1.0}, 1.0, false, text);
}

void render_text(float x, float y, vec3 color, string text)
{
   render_text("consola12", x, y, color, 1.0, false, text);
}

void render_text(string font, float x, float y, bool center, string text) 
{
   render_text(font, x, y, vec3{1.0, 1.0, 1.0}, 1.0, center, text);
}

void render_text(string font, float x, float y, vec3 color, string text) 
{
   render_text(font, x, y, color, 1.0, false, text);
}

void render_text(string font, float x, float y, vec3 color, bool center, string text) 
{
   render_text(font, x, y, color, 1.0, center, text);
}

void render_text(string font, float x, float y, float scale, string text) 
{
   render_text(font, x, y,  vec3{1.0, 1.0, 1.0}, scale, false, text);
}

void render_text(string font, float x, float y, vec3 color, float scale, string text) 
{
   render_text(font, x, y, color, scale, false, text);
}

void render_text(string font, float x, float y, vec3 color, float scale, bool center, string text) 
{
   // Finds text shader in catalogue and set variables 
   auto text_shader = Shader_Catalogue.find("text")->second;
	text_shader->use();
	text_shader->setFloat3("textColor", color.x, color.y, color.z);

   // Finds text drawing geometry in geometry catalogue
   auto text_geometry = Geometry_Catalogue.find("text")->second;
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(text_geometry->gl_data.VAO);

   // Try finding font in catalogue, if doesn't find, tries loading it
   gl_charmap charmap;
   auto font_query = Font_Catalogue.find(font);
   if(font_query == Font_Catalogue.end())
   {
      // search for font size in font name (e.g. Consola12) and loads it
      int ind = 0;
      while(true)
      {
         if(!isalpha(font[ind]))
            break;
         
         else if(ind + 1 == font.size())
         {
            cout << "Font '" << font << "' could not be loaded because no size was "
               << "appended to its name in render_text function call.";
            return;
         }
         ind++;
      }

      string size_str = font.substr(ind, font.size());
      string font_filename  = font.substr(0, ind) + ".ttf";

      int font_size = std::stoi(size_str);
      charmap = load_text_textures(font_filename, font_size);
   }
   else
   {
      charmap = font_query->second;
   }

   //@todo add enum to CENTER, LEFT ALIGN (default, no extra work) and RIGHT ALIGN
   if(center)
   {
      string::iterator it;
      float x_sum = 0;
	   for (it = text.begin(); it != text.end(); it++)
      {
		   auto ch = charmap[*it];
         x_sum += ch.Bearing.x * scale + ch.Size.x * scale;
      }
      x -= x_sum / 2.0;
   }


	std::string::iterator c;
	for (c = text.begin(); c != text.end(); c++) 
   {
		Character ch = charmap[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
         { xpos, ypos + h, 0.0, 0.0 },
         { xpos, ypos, 0.0, 1.0 },
         { xpos + w, ypos, 1.0, 1.0 },
         { xpos, ypos + h, 0.0, 0.0 },
         { xpos + w, ypos, 1.0, 1.0 },
         { xpos + w, ypos + h, 1.0, 0.0 }
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, text_geometry->gl_data.VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
}


// -------------------------
// MESSAGE BUFFER RENDERING
// -------------------------
void render_message_buffer_contents()
{
   //@todo make disappearing effect
   int render_count = 0;
   size_t size = G_BUFFERS.rm_buffer->size;
   auto item = G_BUFFERS.rm_buffer->buffer;
   for(int i = 0; i < size; i++)
   {
      if(render_count == 3)
         break;

      if(item->message != "")
      {
         render_count++;
         render_text(
            "consola20",
            G_DISPLAY_INFO.VIEWPORT_WIDTH / 2, 
            G_DISPLAY_INFO.VIEWPORT_HEIGHT - 120 - render_count * 25,  
            vec3(0.8, 0.8, 0.2),
            true,
            item->message
         );
         item->elapsed += G_FRAME_INFO.duration * 1000.0;
      }

      item++;
   }
}