#pragma once

struct Entity;

const unsigned int   R_SHADOW_BUFFER_WIDTH = 1920, R_SHADOW_BUFFER_HEIGHT = 1080;
unsigned int         R_DEPTH_MAP_FBO;
unsigned int         R_DEPTH_MAP;

mat4                 R_DIR_LIGHT_SPACE_MATRIX;
vec3                 R_DIRECTIONAL_LIGHT_POS = vec3{-2.0f, 4.0f, -1.0f};

// depth cubemap (point light shadow)
unsigned int         R_DEPTH_CUBEMAP_FBO;
mat4                 R_POINT_LIGHT_SPACE_MATRICES[6];
unsigned int         R_DEPTH_CUBEMAP_TEXTURE;
float                R_CUBEMAP_NEAR_PLANE = 1.0f;
float                R_CUBEMAP_FAR_PLANE = 25.0f;


void render_entity               (Entity* entity);
void render_scene                (World* world, Camera* camera);
void render_editor_entity        (Entity* entity, World* world, Camera* camera);
void set_shader_light_variables  (World* world, Shader* shader, Camera* camera);

// --------------
// RENDER ENTITY
// --------------
void render_entity(Entity* entity)
{
   entity->shader->use();
   entity->shader->setMatrix4("model", entity->matModel);

   // bind appropriate textures
   u32 diffuse_n = 1;
   u32 specular_n = 1;
   u32 normal_n = 1;
   u32 height_n = 1;

   u32 i;
   for (i = 0; i < entity->textures.size(); i++)
   {
      // active proper texture unit before binding
      glActiveTexture(GL_TEXTURE0 + i); 
      std::string number;
      // @todo: can turn this into enum for faster int comparison
      std::string type = entity->textures[i].type;
      if       (type == "texture_diffuse")
         number = std::to_string(diffuse_n++);
      else if  (type == "texture_specular")
         number = std::to_string(specular_n++);
      else if  (type == "texture_normal")
         number = std::to_string(normal_n++); 
      else if  (type == "texture_height")
         number = std::to_string(height_n++); 

      // now set the sampler to the correct texture unit
      glUniform1i(glGetUniformLocation(entity->shader->gl_programId, (type + number).c_str()), i);
      // and finally bind the texture
      glBindTexture(GL_TEXTURE_2D, entity->textures[i].id);
   }

   // SHADOW MAPS
   {
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
   }

   // check for tiled texture
   if(entity->flags & EntityFlags_RenderTiledTexture)
   {
      entity->shader->setInt("texture_wrap_top",      entity->uv_tile_wrap[0]);
      entity->shader->setInt("texture_wrap_bottom",   entity->uv_tile_wrap[1]);
      entity->shader->setInt("texture_wrap_front",    entity->uv_tile_wrap[2]);
      entity->shader->setInt("texture_wrap_left",     entity->uv_tile_wrap[3]);
      entity->shader->setInt("texture_wrap_right",    entity->uv_tile_wrap[4]);
      entity->shader->setInt("texture_wrap_back",     entity->uv_tile_wrap[5]);
   }

   if(entity->type == EntityType_TimerMarking)
   {
      entity->shader->setFloat3("color", entity->timer_marking_data.color);
   }

   // draw mesh
   RenderOptions render_opts;
   render_opts.wireframe = entity->flags & EntityFlags_RenderWireframe || entity->flags & EntityFlags_HiddenEntity;
   render_mesh(entity->mesh, render_opts);

   // always good practice to set everything back to defaults once configured.
   glActiveTexture(GL_TEXTURE0);
}


void render_editor_entity(Entity* entity, World* world, Camera* camera)
{
   entity->shader->use();
   // important that the gizmo dont have a position set.
   entity->shader->setMatrix4("model",             entity->matModel);
   entity->shader->setMatrix4("view",              camera->View4x4);
   entity->shader->setMatrix4("projection",        camera->Projection4x4);
   entity->shader->setMatrix4("model",             entity->matModel);
   entity->shader->setFloat3 ("viewPos",           camera->Position);
   entity->shader->setFloat3 ("entity_position",   entity->position);
   entity->shader->setFloat  ("shininess",         world->global_shininess);

   render_entity(entity);
}


// -------------
// RENDER SCENE
// -------------
void render_scene(World* world, Camera* camera) 
{
   // set shader settings that are common to the scene
   // both to "normal" model shader and to tiled model shader
   auto model_shader = Shader_Catalogue.find("model")->second;
   set_shader_light_variables(world, model_shader, camera);

   auto model_tiled_shader = Shader_Catalogue.find("tiledTextureModel")->second;
   set_shader_light_variables(world, model_tiled_shader, camera);

   auto color_shader = Shader_Catalogue.find("color")->second;
   set_shader_light_variables(world, color_shader, camera);

	for(int i = 0; i < world->entities.size(); i++) 
   {
	   Entity* entity = world->entities[i];
      if(entity->flags & EntityFlags_InvisibleEntity)
         continue;

      render_entity(entity);
	}
}


void set_shader_light_variables(World* world, Shader* shader, Camera* camera)
{
   shader->use();

   int light_count;
   // point lights
   {
      light_count = 0;
      for (auto& light : world->point_lights)
      {
         auto uniform_name = "pointLights[" + std::to_string(light_count) + "]";
         shader->setFloat3(uniform_name + ".position",  light->position);
         shader->setFloat3(uniform_name + ".diffuse",   light->diffuse);
         shader->setFloat3(uniform_name + ".specular",  light->specular);
         shader->setFloat(uniform_name  + ".constant",  light->intensity_constant);
         shader->setFloat(uniform_name  + ".linear",    light->intensity_linear);
         shader->setFloat(uniform_name  + ".quadratic", light->intensity_quadratic);
         light_count++;
      }
      shader->setInt("num_point_lights", light_count);
   }

   // spot lights
   {
      light_count = 0;
      for (auto& light : world->spot_lights)
      {
         auto uniform_name = "spotLights[" + std::to_string(light_count) + "]";
         shader->setFloat3(uniform_name + ".position",  light->position);
         shader->setFloat3(uniform_name + ".direction", light->direction);
         shader->setFloat3(uniform_name + ".diffuse",   light->diffuse);
         shader->setFloat3(uniform_name + ".specular",  light->specular);
         shader->setFloat(uniform_name  + ".constant",  light->intensity_constant);
         shader->setFloat(uniform_name  + ".linear",    light->intensity_linear);
         shader->setFloat(uniform_name  + ".quadratic", light->intensity_quadratic);
         shader->setFloat(uniform_name  + ".innercone", light->innercone);
         shader->setFloat(uniform_name  + ".outercone", light->outercone);
         light_count++;
      }

      shader->setInt("num_spot_lights", light_count);
   }

   // directional lights
   {
      light_count = 0;
      for (auto& light : world->directional_lights)
      {
         auto uniform_name = "dirLights[" + std::to_string(light_count) + "]";
         shader->setFloat3(uniform_name + ".direction", light->direction);
         shader->setFloat3(uniform_name + ".diffuse",   light->diffuse);
         shader->setFloat3(uniform_name + ".specular",  light->specular);
         light_count++;
      }
      shader->setInt("num_directional_lights", light_count);
   }

   shader->setMatrix4   ("view",               camera->View4x4);
   shader->setMatrix4   ("projection",         camera->Projection4x4);
   shader->setFloat3    ("viewPos",            camera->Position);
   shader->setFloat     ("shininess",          world->global_shininess);
   shader->setFloat3    ("ambient",            world->ambient_light);
   shader->setFloat     ("ambient_intensity",  world->ambient_intensity);
   shader->setMatrix4   ("lightSpaceMatrix",   R_DIR_LIGHT_SPACE_MATRIX);
   shader->setFloat     ("cubemap_far_plane",  R_CUBEMAP_FAR_PLANE);
}

// leave for debugging
std::string p_grab = "Grabbed: ";
int p_floor = -1;

// -------------------------
// RENDER GAME GUI
// -------------------------
void render_game_gui(Player* player)
{
   auto color = player->lives == 2 ? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1};
   render_text("consola42", 25, 75, color, std::to_string(player->lives));

   if(player->grabbing_entity != NULL)
   {  
      p_grab = "Grabbed: ";
      std::string last_grabbed = player->grabbing_entity->name;
      p_grab += "'" + last_grabbed + "'";
   }
   render_text(GlobalDisplayConfig::VIEWPORT_WIDTH - 400, 45, p_grab);

   std::string player_floor = "player floor: ";
   if(player->standing_entity_ptr != NULL) 
   {
      player_floor += player->standing_entity_ptr->name;
      if(p_floor != player->standing_entity_ptr->id)
      {
         p_floor = player->standing_entity_ptr->id;
         std::cout << "new floor: " << p_floor << "\n";
      }
   }
   render_text(GlobalDisplayConfig::VIEWPORT_WIDTH - 400, 60, player_floor);
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
void render_depth_map(World* world)
{
   // setup
   glViewport(0, 0, R_SHADOW_BUFFER_WIDTH, R_SHADOW_BUFFER_HEIGHT);
   glBindFramebuffer(GL_FRAMEBUFFER, R_DEPTH_MAP_FBO);

   glClear(GL_DEPTH_BUFFER_BIT);
   auto depth_shader = Shader_Catalogue.find("depth")->second;
   depth_shader->use();
   depth_shader->setMatrix4("lightSpaceMatrix", R_DIR_LIGHT_SPACE_MATRIX);
   
   for(int it = 0; it < world->entities.size(); it++) 
   {
      Entity* entity = world->entities[0];
      if(entity->flags & EntityFlags_InvisibleEntity)
         continue;

      depth_shader-> setMatrix4("model", entity->matModel);
      render_mesh(entity->mesh, RenderOptions{});
   }

   // de-setup
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glViewport(0, 0, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT);
}

void render_depth_cubemap(World* world)
{
   // for now, testing, we are doing this just for the first point light source
   if(world->point_lights.size() == 0)
      return;

   auto light = world->point_lights[0];

   float aspect = (float)R_SHADOW_BUFFER_WIDTH/ (float)R_SHADOW_BUFFER_HEIGHT;
   mat4 cubemap_proj = glm::perspective(glm::radians(90.0f), aspect, R_CUBEMAP_NEAR_PLANE, R_CUBEMAP_FAR_PLANE); 
   R_POINT_LIGHT_SPACE_MATRICES[0] = 
      cubemap_proj * glm::lookAt(light->position, light->position + vec3(1.0, 0.0, 0.0), vec3(0.0,-1.0, 0.0));
   R_POINT_LIGHT_SPACE_MATRICES[1] = 
      cubemap_proj * glm::lookAt(light->position, light->position + vec3(-1.0, 0.0, 0.0), vec3(0.0,-1.0, 0.0));
   R_POINT_LIGHT_SPACE_MATRICES[2] = 
      cubemap_proj * glm::lookAt(light->position, light->position + vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));
   R_POINT_LIGHT_SPACE_MATRICES[3] = 
      cubemap_proj * glm::lookAt(light->position, light->position + vec3(0.0,-1.0, 0.0), vec3(0.0, 0.0, -1.0));
   R_POINT_LIGHT_SPACE_MATRICES[4] = 
      cubemap_proj * glm::lookAt(light->position, light->position + vec3(0.0, 0.0, 1.0), vec3(0.0,-1.0, 0.0));
   R_POINT_LIGHT_SPACE_MATRICES[5] = 
      cubemap_proj * glm::lookAt(light->position, light->position + vec3(0.0, 0.0, -1.0), vec3(0.0,-1.0, 0.0));

   // setup
   glViewport(0, 0, R_SHADOW_BUFFER_WIDTH, R_SHADOW_BUFFER_HEIGHT);
   glBindFramebuffer(GL_FRAMEBUFFER, R_DEPTH_CUBEMAP_FBO);

   glClear(GL_DEPTH_BUFFER_BIT);
   auto depth_shader = Shader_Catalogue.find("depth_cubemap")->second;
   depth_shader->use();
   
   for (unsigned int i = 0; i < 6; ++i)
      depth_shader->setMatrix4("shadowMatrices[" + std::to_string(i) + "]", R_POINT_LIGHT_SPACE_MATRICES[i]);
   
   depth_shader->setFloat("cubemap_far_plane", R_CUBEMAP_FAR_PLANE);
   depth_shader->setFloat3("lightPos", light->position);

   for(int i = 0; i < world->entities.size(); i++) 
   {
      auto entity = world->entities[i];
      if(entity->flags & EntityFlags_InvisibleEntity)
         continue;

      depth_shader-> setMatrix4("model", entity->matModel);
      render_mesh(entity->mesh, RenderOptions{});
   }

   // de-setup
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glViewport(0, 0, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT);
}

void render_depth_map_debug()
{
	glViewport(0, 0, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT);
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
// 	glViewport(0, 0, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    ConfigureShaderAndMatrices();
//    glBindTexture(GL_TEXTURE_2D, depthMap);
// }