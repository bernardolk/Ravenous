void render_scene(Scene* scene, Camera* camera);
void render_entity(Entity* entity);


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
   glBindVertexArray(entity->mesh.gl_data.VAO);
   if(entity->mesh.render_method == GL_TRIANGLE_STRIP) 
   {
      glDrawArrays(GL_TRIANGLE_STRIP, 0, entity->mesh.vertices.size());
   }
   else if(entity->mesh.render_method == GL_LINE_LOOP)
   {
      glDrawArrays(GL_LINE_LOOP, 0, entity->mesh.vertices.size());
   }
   else
   {
      glDrawElements(GL_TRIANGLES,  entity->mesh.indices.size(), GL_UNSIGNED_INT, 0);
   }  
   glBindVertexArray(0);

   // always good practice to set everything back to defaults once configured.
   glActiveTexture(GL_TEXTURE0);
}


void render_scene(Scene* scene, Camera* camera) 
{
	Entity **entity_iterator = &(scene->entities[0]);
   int entities_vec_size =  scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   Entity *entity_ptr = *entity_iterator;
		entity_ptr->shader->use();
		auto point_light_ptr = scene->pointLights.begin();
		int point_light_count = 0;
		for (point_light_ptr; point_light_ptr != scene->pointLights.end(); point_light_ptr++)
      {
			PointLight point_light = *point_light_ptr;
			string uniform_name = "pointLights[" + to_string(point_light_count) + "]";
			entity_ptr->shader->setFloat3(uniform_name + ".position", point_light.position);
			entity_ptr->shader->setFloat3(uniform_name + ".diffuse", point_light.diffuse);
			entity_ptr->shader->setFloat3(uniform_name + ".specular", point_light.specular);
			entity_ptr->shader->setFloat3(uniform_name + ".ambient", point_light.ambient);
			entity_ptr->shader->setFloat(uniform_name + ".constant", point_light.intensity_constant);
			entity_ptr->shader->setFloat(uniform_name + ".linear", point_light.intensity_linear);
			entity_ptr->shader->setFloat(uniform_name + ".quadratic", point_light.intensity_quadratic);
			point_light_count++;
		}
		entity_ptr->shader->setInt    ("num_point_lights", point_light_count);
		entity_ptr->shader->setInt    ("num_directional_light", 0);
		entity_ptr->shader->setInt    ("num_spot_lights", 0);
		entity_ptr->shader->setMatrix4("view", camera->View4x4);
		entity_ptr->shader->setMatrix4("projection", camera->Projection4x4);
		entity_ptr->shader->setFloat  ("shininess", scene->global_shininess);
		entity_ptr->shader->setFloat3 ("viewPos", camera->Position);
		//mat4 model_matrix = scale   (mat4identity, vec3(0.01,0.01,0.01));
		entity_ptr->shader->setMatrix4("model", entity_ptr->matModel);
		render_entity(entity_ptr);

      entity_iterator++;
	}
}









