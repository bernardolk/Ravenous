bool load_scene_from_file(std::string scene_name, WorldStruct* world);
Entity* parse_and_load_entity(Parser::Parse p, ifstream* reader, int& line_count, std::string path);
void parse_and_load_player_attribute(Parser::Parse p, ifstream* reader, int& line_count, std::string path, Player* player);
void setup_scene_boilerplate_stuff();
bool save_player_position_to_file(string scene_name, Player* player);
bool save_scene_to_file(string scene_name, Player* player, bool do_copy);
void parse_and_load_light_source(Parser::Parse p, ifstream* reader, int& line_count, string path);
void parse_and_load_camera_settings(Parser::Parse p, ifstream* reader, int& line_count, std::string path);
void parse_and_load_player_orientation(Parser::Parse p, ifstream* reader, int& line_count, std::string path);
bool load_player_attributes_from_file();
bool check_if_scene_exists();
Entity* create_player_entity();
Player* create_player(Entity* player_entity);
ProgramConfig load_configs();
bool save_configs_to_file();

#include <iomanip>


bool load_scene_from_file(std::string scene_name, WorldStruct* world)
{
   string path = SCENES_FOLDER_PATH + scene_name + ".txt";
   ifstream reader(path);

   if(!reader.is_open())
   {
      cout << "Cant load scene from file '" + path + "', path NOT FOUND \n";  
      return false;
   }

   // clears the current scene entity data
   if(G_SCENE_INFO.active_scene != NULL)
      G_SCENE_INFO.active_scene->entities.clear();
      // CLEAR BUFFERS ?

   // Gets a new world struct from scratch
   World.init();

   // creates new scene
   // @todo: possibly leaking memory if switching between scenes.
   auto scene = new Scene();
   G_SCENE_INFO.active_scene = scene;
   G_SCENE_INFO.camera = G_SCENE_INFO.views[0];    // sets to editor camera
   Entity_Manager.set_entity_registry(&G_SCENE_INFO.active_scene->entities);
   Entity_Manager.set_checkpoints_registry(&G_SCENE_INFO.active_scene->checkpoints);

   // creates player
   auto player_entity = create_player_entity();
   auto player = create_player(player_entity);
   scene->entities.push_back(player_entity);
   G_SCENE_INFO.player = player;

   // starts reading
   std::string line;
   Parser::Parse p;
   int line_count = 0;

   // parses entity
   while(parser_nextline(&reader, &line, &p))
   {
      line_count++;
      p = parse_symbol(p);
      if(p.cToken == '#')
      {
         Entity* new_entity = parse_and_load_entity(p, &reader, line_count, path);
         world->update_entity_world_cells(new_entity);
      }
      else if(p.cToken == '@')
      {
         parse_and_load_player_attribute(p, &reader, line_count, path, G_SCENE_INFO.player);
      }
      else if(p.cToken == '$')
      {
         parse_and_load_light_source(p, &reader, line_count, path);
      }
      else if(p.cToken == '*')
      {
         parse_and_load_camera_settings(p, &reader, line_count, path);
      }
      else if(p.cToken == '&')
      {
         parse_and_load_player_orientation(p, &reader, line_count, path);
      }
   }
   
   world->update_cells_in_use_list();

   G_SCENE_INFO.scene_name = scene_name;

   // SAVE BACKUP
   save_scene_to_file("backup", player, true);

   return true;
} 

bool load_player_attributes_from_file(string scene_name, Player* player)
{
   string path = SCENES_FOLDER_PATH + scene_name + ".txt";
   ifstream reader(path);

   if(!reader.is_open())
   {
      cout << "WARNING: Loading player attributes from file failed.\n";
      return false;
   }

   // starts reading
   std::string line;
   Parser::Parse p;
   int line_count = 0;

   while(parser_nextline(&reader, &line, &p))
   {
      line_count++;
      p = parse_symbol(p);
      if(p.cToken == '@')
      {
         parse_and_load_player_attribute(p, &reader, line_count, path, G_SCENE_INFO.player);
      }
   }

   reader.close();
   return true;
}

bool save_scene_to_file(string scene_name, Player* player, bool do_copy)
{
   bool was_renamed = scene_name.length() > 0;
   if(!was_renamed)
      scene_name = G_SCENE_INFO.scene_name;

   if(do_copy && !was_renamed)
   {
      cout << "please provide a name for the copy.\n";
      return false;
   }

   string path = SCENES_FOLDER_PATH + scene_name + ".txt";

   ofstream writer(path);
   if(!writer.is_open())
   {
      cout << "Saving scene failed.\n";
      return false;
   }

   writer << fixed << setprecision(4);

   // write camera settings to file
   auto camera = G_SCENE_INFO.views[0];
   writer << "*" 
      << camera->Position.x << " "
      << camera->Position.y << " "
      << camera->Position.z << "  "
      << camera->Front.x << " "
      << camera->Front.y << " "
      << camera->Front.z << "\n";

   // write player attributes to file
   writer << "@player_position = " 
               << player->entity_ptr->position.x << " " 
               << player->entity_ptr->position.y << " "
               << player->entity_ptr->position.z << "\n";
   writer << "@player_initial_velocity = "
               << player->initial_velocity.x << " " 
               << player->initial_velocity.y << " "
               << player->initial_velocity.z << "\n";

   if(player->player_state == PLAYER_STATE_STANDING)
      writer << "@player_state = " << PLAYER_STATE_STANDING << "\n"; 
   else
      writer << "@player_state = " << player->initial_player_state << "\n"; 

   writer << "@player_fall_acceleration = " << player->fall_acceleration << "\n";
   auto fps_cam = G_SCENE_INFO.views[FPS_CAM];
   writer << "&player_orientation = "
      << fps_cam->Front.x << " "
      << fps_cam->Front.y << " "
      << fps_cam->Front.z << "\n"; 

   // write light sources POINT
   for(int it = 0; it < G_SCENE_INFO.active_scene->pointLights.size(); it++)
   {
      auto light = G_SCENE_INFO.active_scene->pointLights[it];

      writer << "\n$point\n"
            << "position "
            << light.position.x << " "
            << light.position.y << " "
            << light.position.z << "\n"
            << "diffuse "
            << light.diffuse.x << " "
            << light.diffuse.y << " "
            << light.diffuse.z << "\n"
            << "specular "
            << light.specular.x << " "
            << light.specular.y << " "
            << light.specular.z << "\n"
            << "constant "
            << light.intensity_constant << "\n"
            << "linear "
            << light.intensity_linear << "\n"
            << "quadratic "
            << light.intensity_quadratic << "\n";
   }

   // write light sources SPOT
   for(int it = 0; it < G_SCENE_INFO.active_scene->spotLights.size(); it++)
   {
      auto light = G_SCENE_INFO.active_scene->spotLights[it];

      writer << "\n$spot\n"
            << "position "
            << light.position.x << " "
            << light.position.y << " "
            << light.position.z << "\n"
            << "direction "
            << light.direction.x << " "
            << light.direction.y << " "
            << light.direction.z << "\n"
            << "diffuse "
            << light.diffuse.x << " "
            << light.diffuse.y << " "
            << light.diffuse.z << "\n"
            << "specular "
            << light.specular.x << " "
            << light.specular.y << " "
            << light.specular.z << "\n"
            << "innercone "
            << light.innercone << "\n"
            << "outercone "
            << light.outercone << "\n"
            << "constant "
            << light.intensity_constant << "\n"
            << "linear "
            << light.intensity_linear << "\n"
            << "quadratic "
            << light.intensity_quadratic << "\n";
   }

   // write light sources DIRECTIONAL
   for(int it = 0; it < G_SCENE_INFO.active_scene->directionalLights.size(); it++)
   {
      auto light = G_SCENE_INFO.active_scene->directionalLights[it];

      writer << "\n$directional\n"
            << "direction "
            << light.direction.x << " "
            << light.direction.y << " "
            << light.direction.z << "\n"
            << "diffuse "
            << light.diffuse.x << " "
            << light.diffuse.y << " "
            << light.diffuse.z << "\n"
            << "specular "
            << light.specular.x << " "
            << light.specular.y << " "
            << light.specular.z << "\n";
   }

   // write scene data (for each entity)
   Entity **entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;
      if(entity->name == "Player")
         continue;

      writer << "\n#" << entity->name << "\n";
      writer << "position " 
               << entity->position.x << " "
               << entity->position.y << " "
               << entity->position.z << "\n";
      writer << "rotation " 
               << entity->rotation.x << " "
               << entity->rotation.y << " "
               << entity->rotation.z << "\n";
      writer << "scale " 
               << entity->scale.x << " "
               << entity->scale.y << " "
               << entity->scale.z << "\n";
      writer << "mesh " << entity->mesh->name << "\n";
      writer << "shader " << entity->shader->name << "\n";

      int textures =  entity->textures.size();
      for(int t = 0; t < textures; t++)
      {
         Texture texture = entity->textures[t];
         writer << "texture " 
                  << texture.type << " "
                  << texture.name << " "
                  << texture.path << "\n";
      }

      writer << "collision ";
      if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
         writer << "aabb\n";
      else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
         writer << "slope\n";
      else
         assert(false);
      
      if(entity->wireframe)
         writer << "hidden\n";

      if(entity->type == CHECKPOINT)
      {
         writer << "type 1\n";
         writer << "trigger " 
            << entity->trigger_scale.x << " "
            << entity->trigger_scale.y << " "
            << entity->trigger_scale.z << "\n";
      }
   }

   // Write things that need to be written after all entities are saved
   writer << "\n";
   if(player->player_state == PLAYER_STATE_STANDING)
      writer << "@player_standing_entity = " << player->standing_entity_ptr->name << "\n";

   writer.close();

   if(do_copy)
      cout << "Scene copy saved succesfully as '" << scene_name << ".txt'. \n";
   else if(was_renamed)
   {
      cout << "Scene saved succesfully as '" << scene_name << ".txt' (now editing it). \n";
      G_SCENE_INFO.scene_name = scene_name;
   }
   else
      cout << "Scene saved succesfully.\n";

   return true;
}


void parse_and_load_camera_settings(Parser::Parse p, ifstream* reader, int& line_count, std::string path)
{
      p = parse_all_whitespace(p);
      p = parse_float_vector(p);
      G_SCENE_INFO.camera->Position = vec3{p.vec3[0], p.vec3[1], p.vec3[2]};

      p = parse_all_whitespace(p);
      p = parse_float_vector(p);
      camera_look_at(G_SCENE_INFO.camera, vec3{p.vec3[0], p.vec3[1], p.vec3[2]}, false);
}

void parse_and_load_player_orientation(Parser::Parse p, ifstream* reader, int& line_count, std::string path)
{
   p = parse_token(p);
   std::string attribute = p.string_buffer;

   p = parse_all_whitespace(p);
   p = parse_symbol(p);

   if(p.cToken != '=')
   {
      std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << path << "') LINE NUMBER " << line_count << "\n";
      assert(false);
   }

   if(attribute == "player_orientation")
   {
      p = parse_float_vector(p);
      auto orientation = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      G_SCENE_INFO.views[FPS_CAM]->Front = orientation;
   }
}

void parse_and_load_player_attribute(Parser::Parse p, ifstream* reader, int& line_count, std::string path, Player* player)
{
   p = parse_token(p);
   std::string attribute = p.string_buffer;

   p = parse_all_whitespace(p);
   p = parse_symbol(p);

   if(p.cToken != '=')
   {
      std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << path << "') LINE NUMBER " << line_count << "\n";
      assert(false);
   }

   if(attribute == "player_position")
   {
      p = parse_float_vector(p);
      auto position = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      player->entity_ptr->position = position;
      player->checkpoint_pos = position;
      player->height_before_fall = position.y;
   }
   else if(attribute == "player_initial_velocity")
   {
      p = parse_float_vector(p);
      player->initial_velocity = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      player->entity_ptr->velocity = player->initial_velocity;
   }
   else if(attribute == "player_standing_entity")
   {
      std::string entity_name;
      p = parse_all_whitespace(p);
      p = parse_token(p);
      entity_name = p.string_buffer;

      auto entity_ptr = G_SCENE_INFO.active_scene->find_entity(entity_name);
      if(entity_ptr != NULL)
         player->standing_entity_ptr = entity_ptr;
      else
      {
         cout << "COULDN'T FIND PLAYER_STANDING_ENTITY IN SCENE ENTITIES." << 
            "MAKE SURE THIS ATTRIBUTE IS LOADED AFTER ALL ENTITIES ARE LOADED\n";
         assert(false);
      }
   }
   else if(attribute == "player_state")
   {
      p = parse_all_whitespace(p);
      p = parse_int(p);
      player->initial_player_state = (PlayerStateEnum) p.iToken;
      player->player_state = player->initial_player_state;
   }
   else if(attribute == "player_fall_speed")
   {
      p = parse_all_whitespace(p);
      p = parse_float(p);

      player->fall_speed = p.fToken;
   }
   else if(attribute == "player_fall_acceleration")
   {
      p = parse_all_whitespace(p);
      p = parse_float(p);

      player->fall_acceleration = p.fToken;
   }
   else
   {
      std::cout << "UNRECOGNIZED ATTRIBUTE AT SCENE DESCRIPTION FILE ('" << path << "') LINE NUMBER " << line_count << "\n";
      assert(false);
   }
}


Entity* parse_and_load_entity(Parser::Parse p, ifstream* reader, int& line_count, std::string path)
{
   std::string line;
   bool is_collision_parsed = false;

   auto new_entity = Entity_Manager.create_entity(false);
   p = parse_name(p);
   new_entity->name = p.string_buffer;

   while(parser_nextline(reader, &line, &p))
   {
      line_count ++;
      p = parse_token(p);
      const std::string property = p.string_buffer;
      if(property == "position")
      {
            p = parse_float_vector(p);
            new_entity->position = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      }
      else if(property == "rotation")
      {
            p = parse_float_vector(p);
            new_entity->rotation = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      }
      else if(property == "scale")
      {
            if(is_collision_parsed)
            {
               std::cout << "FATAL: COLLISION SHOULD BE DEFINED AFTER SCALE PROPERTY FOR ENTITY. AT '" << path 
                         << "' LINE NUMBER " << line_count << "\n";
               assert(false);
            }
            p = parse_float_vector(p);
            if(p.vec3[0] < 0 || p.vec3[1] < 0 || p.vec3[2] < 0)
            {
               std::cout << "FATAL: ENTITY SCALE PROPERTY CANNOT BE NEGATIVE. AT '" << path
                         << "' LINE NUMBER " << line_count << "\n";
            }
            new_entity->scale = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      }
      else if(property == "shader")
      {
         std::string shader_name;
         p = parse_all_whitespace(p);
         p = parse_name(p);
         shader_name = p.string_buffer;

         auto find = Shader_Catalogue.find(shader_name);
         if(find != Shader_Catalogue.end())
         {
            new_entity->shader = find->second;
         }
         else
         {
            std::cout << "SHADER '" << shader_name << "' NOT FOUND WHILE LOADING SCENE DESCRIPTION FILE \n"; 
            assert(false);
         }   
      }
      else if(property == "mesh")
      {
         std::string model_name;
         p = parse_all_whitespace(p);
         p = parse_token(p);
         model_name = p.string_buffer;

         auto find = Geometry_Catalogue.find(model_name);
         if(find != Geometry_Catalogue.end())
            new_entity->mesh = find->second;
         else
            new_entity->mesh = load_wavefront_obj_as_mesh(MODELS_PATH, model_name);

         // makes collision mesh equals to mesh
         new_entity->collision_mesh = new_entity->mesh;
      }
      else if(property == "texture")
      {
         std::string texture_type, texture_name, texture_filename;
         p = parse_all_whitespace(p);
         p = parse_token(p);
         texture_type = p.string_buffer;

         p = parse_all_whitespace(p);
         p = parse_token(p);
         texture_name = p.string_buffer;

         p = parse_all_whitespace(p);
         p = parse_token(p);
         texture_filename = p.string_buffer;

         auto find = Texture_Catalogue.find(texture_name);
         if(find != Texture_Catalogue.end())
         {
            new_entity->textures.push_back(find->second);
         }
         else
         {
            // texture definition error handling
            if(texture_name == "" || texture_type == "" || texture_filename == "")
            {
               std::cout << "Fatal: Texture for entity '" << new_entity->name << "' is missing either name or type or filename. \n"; 
               assert(false);
            }
            if(!(texture_type == "texture_diffuse" || texture_type == "texture_normal"))
            {
               std::cout<<"Fatal: '"<<texture_name<<"' has unknown texture type '" <<texture_type << "'.\n"; 
               assert(false);
            }

            unsigned int texture_id = load_texture_from_file(texture_filename, TEXTURES_PATH);
            if(texture_id == 0)
            {
               cout << "Texture '" << texture_name << "' could not be loaded. \n"; 
               assert(false);
            }
            
            Texture new_texture{texture_id, texture_type, texture_filename, texture_name};
            Texture_Catalogue.insert({texture_name, new_texture});
            new_entity->textures.push_back(new_texture);
         }   
      }
      else if(property == "collision")
      {
         // check so we make sure scale is defined before collision (we use it to define the aabb lengths)
         is_collision_parsed = true;

         p = parse_all_whitespace(p);
         p = parse_token(p);
         string collision_type = p.string_buffer;

         if(collision_type != "aabb" && collision_type != "slope")
         {
            std::cout << "UNRECOGNIZED COLLISION TYPE '" << collision_type 
                        << "' AT SCENE DESCRIPTION FILE ('" 
                        << path << "') LINE NUMBER " << line_count << "\n";
            assert(false);
         }

         if(collision_type == "aabb")
         {
            new_entity->collision_geometry_type = COLLISION_ALIGNED_BOX;
         }
         else if(collision_type == "slope")
         {
            new_entity->collision_geometry_type = COLLISION_ALIGNED_SLOPE;
         }
         
         new_entity->update_collision_geometry();
      }
      else if(property == "hidden")
      {
         new_entity->wireframe = true;
      }
      else if(property == "type")
      {
         p = parse_all_whitespace(p);
         p = parse_int(p);
         int entity_type = p.iToken;
         auto type_enum = (EntityType) entity_type;
         Entity_Manager.set_type(new_entity, type_enum);
      }
      else if(property == "trigger")
      {
         p = parse_float_vector(p);
         new_entity->trigger_scale = vec3{p.vec3[0], p.vec3[1], p.vec3[2]};
      }
      else
      {
         break;
      }
   }
   return new_entity;
}

void parse_and_load_light_source(Parser::Parse p, ifstream* reader, int& line_count, string path)
{
   string line;

   p = parse_token(p);
   string type = p.string_buffer;

   if (!(type == "point" || type == "spot" || type == "directional"))
   {
      cout << "FATAL: Unrecognized light source in scene file '" << path << "', line " << line_count << ".\n";
      assert(false);
   }

   if(type == "point")
   {
      PointLight point_light;

      while(parser_nextline(reader, &line, &p))
      {
         line_count++;
         p = parse_token(p);
         const std::string property = p.string_buffer;

         if(property == "position")
         {
            p = parse_float_vector(p);
            point_light.position = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "diffuse")
         {
            p = parse_float_vector(p);
            point_light.diffuse = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "specular")
         {
            p = parse_float_vector(p);
            point_light.specular = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "constant")
         {
            p = parse_all_whitespace(p);
            p = parse_float(p);
            point_light.intensity_constant = p.fToken;
         }
         else if(property == "linear")
         {
            p = parse_all_whitespace(p);
            p = parse_float(p);
            point_light.intensity_linear = p.fToken;
         }
         else if(property == "quadratic")
         {
            p = parse_all_whitespace(p);
            p = parse_float(p);
            point_light.intensity_quadratic = p.fToken;
         }
         else break;
      }

      G_SCENE_INFO.active_scene->pointLights.push_back(point_light);
   }
   else if(type == "spot")
   {
      SpotLight spotlight;

      while(parser_nextline(reader, &line, &p))
      {
         line_count++;
         p = parse_token(p);
         const std::string property = p.string_buffer;

         if(property == "position")
         {
            p = parse_float_vector(p);
            spotlight.position = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "direction")
         {
            p = parse_float_vector(p);
            spotlight.direction = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "diffuse")
         {
            p = parse_float_vector(p);
            spotlight.diffuse = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "specular")
         {
            p = parse_float_vector(p);
            spotlight.specular = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "constant")
         {
            p = parse_all_whitespace(p);
            p = parse_float(p);
            spotlight.intensity_constant = p.fToken;
         }
         else if(property == "linear")
         {
            p = parse_all_whitespace(p);
            p = parse_float(p);
            spotlight.intensity_linear = p.fToken;
         }
         else if(property == "quadratic")
         {
            p = parse_all_whitespace(p);
            p = parse_float(p);
            spotlight.intensity_quadratic = p.fToken;
         }
         else if(property == "innercone")
         {
            p = parse_all_whitespace(p);
            p = parse_float(p);
            spotlight.innercone = p.fToken;
         }
         else if(property == "outercone")
         {
            p = parse_all_whitespace(p);
            p = parse_float(p);
            spotlight.outercone = p.fToken;
         }
         else break;
      }

      G_SCENE_INFO.active_scene->spotLights.push_back(spotlight);     
   }
   else if(type == "directional")
   {
      DirectionalLight light;

      while(parser_nextline(reader, &line, &p))
      {
         line_count++;
         p = parse_token(p);
         const std::string property = p.string_buffer;

         if(property == "direction")
         {
            p = parse_float_vector(p);
            light.direction = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "diffuse")
         {
            p = parse_float_vector(p);
            light.diffuse = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "specular")
         {
            p = parse_float_vector(p);
            light.specular = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else break;
      }

      G_SCENE_INFO.active_scene->directionalLights.push_back(light);     
   }
}

bool save_player_position_to_file(string scene_name, Player* player)
{
   // NOT IMPLEMENTED
   return false;
}


Entity* create_player_entity()
{
   // cylinder
   auto find1 = Shader_Catalogue.find("model");
   auto model_shader = find1->second;

   auto find2 = Geometry_Catalogue.find("quad");
   auto quad_mesh = find2->second;

   auto find_cylinder = Geometry_Catalogue.find("cylinder");
   auto cylinder_mesh = find_cylinder->second;

   auto entity = Entity_Manager.create_entity();
   entity->name = "Player";
   entity->shader = model_shader;

   unsigned int pink_texture = load_texture_from_file("pink.jpg", TEXTURES_PATH);
   entity->textures = 
      std::vector<Texture> {
         Texture {
            pink_texture,
            "texture_diffuse",
            "whatever"
         }
      };
   entity->mesh = cylinder_mesh;
   entity->collision_mesh = cylinder_mesh;

   // player collision geometry
   auto cgac = new CollisionGeometryAlignedCylinder { P_HALF_HEIGHT, P_RADIUS };
   entity->collision_geometry_type = COLLISION_ALIGNED_CYLINDER;
   entity->collision_geometry.cylinder = *cgac;

   // player scale
   entity->scale = vec3{P_RADIUS, P_HALF_HEIGHT, P_RADIUS};

   return entity;
}

Player* create_player(Entity* player_entity)
{
   auto player = new Player();
   player->entity_ptr   = player_entity;
   player->half_height  = P_HALF_HEIGHT;
   player->radius       = P_RADIUS;
   return player;
}

bool check_if_scene_exists(string scene_name)
{
   string path = SCENES_FOLDER_PATH + scene_name + ".txt";
   ifstream reader(path);
   return reader.is_open();
}

ProgramConfig load_configs()
{
   ifstream reader(CONFIG_FILE_PATH);

   if(!reader.is_open())
   {
      cout << "FATAL: Cant load config file '" + CONFIG_FILE_PATH + "', path NOT FOUND \n";  
      assert(false);
   }

   auto config = ProgramConfig();

   // starts reading
   std::string line;
   Parser::Parse p;
   int line_count = 0;

   while(parser_nextline(&reader, &line, &p))
   {
      line_count++;
      p = parse_token(p);
      string attribute = p.string_buffer;

      p = parse_all_whitespace(p);
      p = parse_symbol(p);
      if(p.cToken != '=')
      {
         std::cout << 
            "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << 
            CONFIG_FILE_PATH  << 
            "') LINE NUMBER " << 
            line_count << "\n";
            
         assert(false);
      }
      
     
      if(attribute == "scene")
      {
         p = parse_all_whitespace(p);
         p = parse_token(p);
         config.initial_scene = p.string_buffer;
      }
      else if(attribute == "camspeed")
      {
         p = parse_all_whitespace(p);
         p = parse_float(p);
         config.camspeed = p.fToken;
      }
      else if(attribute == "ambient_light")
      {
         p = parse_all_whitespace(p);
         p = parse_float_vector(p);
         config.ambient_light = vec3{p.vec3[0], p.vec3[1], p.vec3[2]};
      }
      else if(attribute == "ambient_intensity")
      {
         p = parse_all_whitespace(p);
         p = parse_float(p);
         config.ambient_intensity = p.fToken;
      }
   }

   return config;
}

bool save_configs_to_file()
{
   ofstream writer(CONFIG_FILE_PATH);
   if(!writer.is_open())
   {
      cout << "Saving config file failed.\n";
      return false;
   }

   writer << "scene = " << G_CONFIG.initial_scene << "\n";
   writer << "camspeed = " << G_CONFIG.camspeed << "\n";
   writer << "ambient_light = " 
      << G_CONFIG.ambient_light.x << " "
      << G_CONFIG.ambient_light.y << " "
      << G_CONFIG.ambient_light.z << "\n";
      
   writer << "ambient_intensity = " << G_CONFIG.ambient_intensity << "\n";

   writer.close();
   cout << "Config file saved succesfully.\n";

   return true;
}