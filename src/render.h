const unsigned int R_SHADOW_BUFFER_WIDTH = 1920, R_SHADOW_BUFFER_HEIGHT = 1080;
unsigned int R_DEPTH_MAP_FBO;
unsigned int R_DEPTH_MAP;
mat4 R_DIR_LIGHT_SPACE_MATRIX;
vec3 R_DIRECTIONAL_LIGHT_POS = vec3{-2.0f, 4.0f, -1.0f};
// depth cubemap (point light shadow)
unsigned int R_DEPTH_CUBEMAP_FBO;
mat4 R_POINT_LIGHT_SPACE_MATRICES[6];
unsigned int R_DEPTH_CUBEMAP_TEXTURE;
float R_CUBEMAP_NEAR_PLANE = 1.0f;
float R_CUBEMAP_FAR_PLANE = 25.0f;

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

   unsigned int i;
   for (i = 0; i < entity->textures.size(); i++)
   {
      // active proper texture unit before binding
      glActiveTexture(GL_TEXTURE0 + i); 
      string number;
      string name = entity->textures[i].type;
      if (name == "texture_diffuse")
         number = to_string(diffuseNr++);
      else if (name == "texture_specular")
         number = to_string(specularNr++);
      else if (name == "texture_normal")
         number = to_string(normalNr++); 
      else if (name == "texture_height")
         number = to_string(heightNr++); 

      // now set the sampler to the correct texture unit
      glUniform1i(glGetUniformLocation(entity->shader->gl_programId, (name + number).c_str()), i);
      // and finally bind the texture
      glBindTexture(GL_TEXTURE_2D, entity->textures[i].id);
   }

   // shadow map texture
   glActiveTexture(GL_TEXTURE0 + i);
   glUniform1i(glGetUniformLocation(entity->shader->gl_programId, "shadowMap"), i);
   glBindTexture(GL_TEXTURE_2D, R_DEPTH_MAP);
   i++;

   // shadow cubemap texture
   glActiveTexture(GL_TEXTURE0 + i);
   glUniform1i(glGetUniformLocation(entity->shader->gl_programId, "shadowCubemap"), i);
   glBindTexture(GL_TEXTURE_CUBE_MAP, R_DEPTH_CUBEMAP_TEXTURE);
   i++;

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
      shader->setFloat(uniform_name  + ".innercone", spotlight.innercone);
      shader->setFloat(uniform_name  + ".outercone", spotlight.outercone);
      spotlight_count++;
   }

   int dir_count = 0;
   for (auto dir_ptr = scene->directionalLights.begin(); 
      dir_ptr != scene->directionalLights.end(); 
      dir_ptr++)
   {
      DirectionalLight dir_light = *dir_ptr;
      string uniform_name = "dirLights[" + to_string(dir_count) + "]";
      shader->setFloat3(uniform_name + ".direction", dir_light.direction);
      shader->setFloat3(uniform_name + ".diffuse",   dir_light.diffuse);
      shader->setFloat3(uniform_name + ".specular",  dir_light.specular);
      dir_count++;
   }

   shader->     setInt("num_directional_lights", dir_count);
   shader->     setInt("num_spot_lights",     spotlight_count);
   shader->     setInt("num_point_lights",    point_light_count);
   shader-> setMatrix4("view",                camera->View4x4);
   shader-> setMatrix4("projection",          camera->Projection4x4);
   shader->   setFloat("shininess",           scene->global_shininess);
   shader->  setFloat3("ambient",             scene->ambient_light);
   shader->   setFloat("ambient_intensity",   scene->ambient_intensity);
   shader->  setFloat3("viewPos",             camera->Position);
   shader-> setMatrix4("lightSpaceMatrix",    R_DIR_LIGHT_SPACE_MATRIX);
   shader->   setFloat("cubemap_far_plane",   R_CUBEMAP_FAR_PLANE);

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


   glDepthFunc(GL_ALWAYS);
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
   glDepthFunc(GL_LESS);
}

// ----------------
// RENDER FEATURES
// ----------------

void create_depth_buffer()
{
   // create framebuffer objects
   glGenFramebuffers(1, &R_DEPTH_MAP_FBO);
   glGenFramebuffers(1, &R_DEPTH_CUBEMAP_FBO);

   // for directional lights:
   // create framebuffer depth buffer texture
   glGenTextures(1, &R_DEPTH_MAP);
   glBindTexture(GL_TEXTURE_2D, R_DEPTH_MAP);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, R_SHADOW_BUFFER_WIDTH, 
      R_SHADOW_BUFFER_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL
   );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);  

   // bind texture to framebuffer
   glBindFramebuffer(GL_FRAMEBUFFER, R_DEPTH_MAP_FBO);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, R_DEPTH_MAP, 0);
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   // for point lights:
   glGenTextures(1, &R_DEPTH_CUBEMAP_TEXTURE);
   glBindTexture(GL_TEXTURE_CUBE_MAP, R_DEPTH_CUBEMAP_TEXTURE);
   for (unsigned int i = 0; i < 6; i++)
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
         R_SHADOW_BUFFER_WIDTH, R_SHADOW_BUFFER_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL
      );  
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

   glBindFramebuffer(GL_FRAMEBUFFER, R_DEPTH_CUBEMAP_FBO);
   glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, R_DEPTH_CUBEMAP_TEXTURE, 0);
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void create_light_space_transform_matrices()
{
   float near_plane, far_plane;

   // directional light matrix
   near_plane = 1.0f;
   far_plane = 7.5f;
   mat4 light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); 
   mat4 light_view = glm::lookAt(
      R_DIRECTIONAL_LIGHT_POS, vec3(0.0f, 0.0f,  0.0f), vec3(0.0f, 1.0f, 0.0f)
   );
   R_DIR_LIGHT_SPACE_MATRIX = light_projection * light_view;
}

// -----------------
// RENDER DEPTH MAP
// -----------------
void render_depth_map()
{
   // setup
   glViewport(0, 0, R_SHADOW_BUFFER_WIDTH, R_SHADOW_BUFFER_HEIGHT);
   glBindFramebuffer(GL_FRAMEBUFFER, R_DEPTH_MAP_FBO);

   glClear(GL_DEPTH_BUFFER_BIT);
   auto depth_shader = Shader_Catalogue.find("depth")->second;
   depth_shader->use();
   depth_shader->setMatrix4("lightSpaceMatrix", R_DIR_LIGHT_SPACE_MATRIX);
   
   Entity **entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
   for(int it = 0; it < entities_vec_size; it++) 
   {
      auto entity = *entity_iterator++;
      if(!entity->render_me)
         continue;

      depth_shader-> setMatrix4("model", entity->matModel);
      render_mesh(entity->mesh, RenderOptions{});
   }

   // de-setup
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glViewport(0, 0, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT);
}

void render_depth_cubemap()
{
   // for now, testing, we are doing this just for the first point light source
   auto& scene = G_SCENE_INFO.active_scene;
   if(scene->pointLights.size() == 0)
      return;

   auto light = scene->pointLights[0];

   float aspect = (float)R_SHADOW_BUFFER_WIDTH/ (float)R_SHADOW_BUFFER_HEIGHT;
   mat4 cubemap_proj = glm::perspective(glm::radians(90.0f), aspect, R_CUBEMAP_NEAR_PLANE, R_CUBEMAP_FAR_PLANE); 
   R_POINT_LIGHT_SPACE_MATRICES[0] = 
      cubemap_proj * glm::lookAt(light.position, light.position + vec3(1.0, 0.0, 0.0), vec3(0.0,-1.0, 0.0));
   R_POINT_LIGHT_SPACE_MATRICES[1] = 
      cubemap_proj * glm::lookAt(light.position, light.position + vec3(-1.0, 0.0, 0.0), vec3(0.0,-1.0, 0.0));
   R_POINT_LIGHT_SPACE_MATRICES[2] = 
      cubemap_proj * glm::lookAt(light.position, light.position + vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));
   R_POINT_LIGHT_SPACE_MATRICES[3] = 
      cubemap_proj * glm::lookAt(light.position, light.position + vec3(0.0,-1.0, 0.0), vec3(0.0, 0.0, -1.0));
   R_POINT_LIGHT_SPACE_MATRICES[4] = 
      cubemap_proj * glm::lookAt(light.position, light.position + vec3(0.0, 0.0, 1.0), vec3(0.0,-1.0, 0.0));
   R_POINT_LIGHT_SPACE_MATRICES[5] = 
      cubemap_proj * glm::lookAt(light.position, light.position + vec3(0.0, 0.0, -1.0), vec3(0.0,-1.0, 0.0));

   // setup
   glViewport(0, 0, R_SHADOW_BUFFER_WIDTH, R_SHADOW_BUFFER_HEIGHT);
   glBindFramebuffer(GL_FRAMEBUFFER, R_DEPTH_CUBEMAP_FBO);

   glClear(GL_DEPTH_BUFFER_BIT);
   auto depth_shader = Shader_Catalogue.find("depth_cubemap")->second;
   depth_shader->use();
   for (unsigned int i = 0; i < 6; ++i)
      depth_shader->setMatrix4("shadowMatrices[" + std::to_string(i) + "]", R_POINT_LIGHT_SPACE_MATRICES[i]);
   depth_shader->setFloat("cubemap_far_plane", R_CUBEMAP_FAR_PLANE);
   depth_shader->setFloat3("lightPos", light.position);

   Entity **entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
   for(int it = 0; it < entities_vec_size; it++) 
   {
      auto entity = *entity_iterator++;
      if(!entity->render_me)
         continue;

      depth_shader-> setMatrix4("model", entity->matModel);
      render_mesh(entity->mesh, RenderOptions{});
   }

   // de-setup
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glViewport(0, 0, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT);
}

void render_depth_map_debug()
{
	glViewport(0, 0, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT);
   //glClearColor(0.196, 0.298, 0.3607, 1.0f);
   //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   // shadow map texture

   
   auto depth_debug_shader = Shader_Catalogue.find("depth_debug")->second;
   depth_debug_shader->use();
   /*
   depth_debug_shader->setInt("depthMap", 0);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, R_DEPTH_MAP);
   auto plane = Geometry_Catalogue.find("plane")->second;
   render_mesh(plane);
   */

   auto aabb = Geometry_Catalogue.find("aabb")->second;
   render_mesh(aabb);
}

// void use_depth_map()
// {
// 	glViewport(0, 0, G_DISPLAY_INFO.VIEWPORT_WIDTH, G_DISPLAY_INFO.VIEWPORT_HEIGHT);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    ConfigureShaderAndMatrices();
//    glBindTexture(GL_TEXTURE_2D, depthMap);
// }