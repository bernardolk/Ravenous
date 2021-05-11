void render_scene(Scene* scene, Camera* camera);
void render_entity(Entity* entity);
void render_text(std::string text, float x, float y, float scale, vec3 color = vec3(1,1,1));


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
   entity->mesh.draw();

   // always good practice to set everything back to defaults once configured.
   glActiveTexture(GL_TEXTURE0);
}


void render_scene(Scene* scene, Camera* camera) 
{
	Entity **entity_iterator = &(scene->entities[0]);
   int entities_vec_size =  scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;

      if(!entity->render_me)
         continue;

      entity->shader->use();
      auto point_light_ptr = scene->pointLights.begin();
      int point_light_count = 0;
      for (point_light_ptr; point_light_ptr != scene->pointLights.end(); point_light_ptr++)
      {
         PointLight point_light = *point_light_ptr;
         string uniform_name = "pointLights[" + to_string(point_light_count) + "]";
         entity->shader->setFloat3(uniform_name + ".position",  point_light.position);
         entity->shader->setFloat3(uniform_name + ".diffuse",   point_light.diffuse);
         entity->shader->setFloat3(uniform_name + ".specular",  point_light.specular);
         entity->shader->setFloat3(uniform_name + ".ambient",   point_light.ambient);
         entity->shader->setFloat(uniform_name  + ".constant",  point_light.intensity_constant);
         entity->shader->setFloat(uniform_name  + ".linear",    point_light.intensity_linear);
         entity->shader->setFloat(uniform_name  + ".quadratic", point_light.intensity_quadratic);
         point_light_count++;
      }
      entity->shader->     setInt("num_directional_light", 0);
      entity->shader->     setInt("num_spot_lights",       0);
      entity->shader->     setInt("num_point_lights",    point_light_count);
      entity->shader-> setMatrix4("view",                camera->View4x4);
      entity->shader-> setMatrix4("projection",          camera->Projection4x4);
      entity->shader->   setFloat("shininess",           scene->global_shininess);
      entity->shader->  setFloat3("viewPos",             camera->Position);
      entity->shader-> setMatrix4("model",               entity->matModel);

      render_entity(entity);
	}
}

void render_game_gui(Player* player)
{
   auto color = player->lives == 2? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1};
   string lives_text = to_string(player->lives);
   render_text(lives_text, 25, 75, 3, color);
}

void render_immediate(GlobalImmediateDraw* im, Camera* camera)
{
   for(int i = 0; i < im->ind; i++)
   {
      auto mesh = im->meshes[i];
      switch(mesh->render_method)
      {
         case GL_POINTS:
         {
            auto find = Shader_Catalogue.find("immediate_point");
            auto shader = find->second;
            shader-> use();
            shader-> setMatrix4("view",        camera->View4x4);
            shader-> setMatrix4("projection",  camera->Projection4x4);
         }
      }
      mesh->draw();
   }
   
   G_IMMEDIATE_DRAW.reset();
}

void render_text(std::string text, float x, float y, float scale, vec3 color) 
{
   auto find1 = Shader_Catalogue.find("text");
   auto text_shader = find1->second;
	text_shader->use();
	text_shader->setFloat3("textColor", color.x, color.y, color.z);

   auto find2 = Geometry_Catalogue.find("text");
   Mesh* text_geometry = find2->second;
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(text_geometry->gl_data.VAO);

	std::string::iterator c;
	for (c = text.begin(); c != text.end(); c++) 
   {
		Character ch = Characters[*c];

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







