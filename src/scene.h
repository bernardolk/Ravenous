
void load_scene_from_file(std::string path, Player* player);
void parse_and_load_entity(Parse p, ifstream* reader, int& line_count, std::string path);
void parse_and_load_attribute(Parse p, ifstream* reader, int& line_count, std::string path, Player* player);



void load_scene_from_file(std::string path, Player* player)
{
   ifstream reader(path);
   std::string line;
   Parse p;
   int line_count = 0;

   // parses entity
   while(parser_nextline(&reader, &line, &p))
   {
      line_count++;
      p = parse_symbol(p);
      if(p.cToken == '#')
      {
         parse_and_load_entity(p, &reader, line_count, path);
      }
      else if(p.cToken == '@')
      {
         parse_and_load_attribute(p, &reader, line_count, path, player);
      }
   }
} 

void parse_and_load_attribute(Parse p, ifstream* reader, int& line_count, std::string path, Player* player)
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

      // find player in active scene's entity list
      int entities_size = G_SCENE_INFO.active_scene->entities.size();
      for (int i = 0; i < entities_size; i ++)
      {
         std::string e_name = G_SCENE_INFO.active_scene->entities[i]->name;
         if (e_name == "player")
         {
            G_SCENE_INFO.active_scene->entities[i]->position = glm::vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
      }
   }
   else if(attribute == "player_velocity")
   {
      p = parse_float_vector(p);

      // find player in active scene's entity list
      int entities_size = G_SCENE_INFO.active_scene->entities.size();
      for (int i = 0; i < entities_size; i ++)
      {
         std::string e_name = G_SCENE_INFO.active_scene->entities[i]->name;
         if (e_name == "player")
         {
            G_SCENE_INFO.active_scene->entities[i]->velocity = glm::vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
      }
   }
   else if(attribute == "player_state")
   {
      p = parse_all_whitespace(p);
      p = parse_int(p);
      switch(p.iToken)
      {
         case 0:
         {
            player->player_state = PLAYER_STATE_FALLING;
            break;
         }
         case 1:
         {
            player->player_state = PLAYER_STATE_STANDING;
            break;
         }
         default:
         {
            std::cout << "UNRECOGNIZED PLAYER STATE AT player_state ATTRIBUTE IN SCENE DESCRIPTION FILE ('" << path << "') LINE NUMBER " << line_count << "\n";
            assert(false);
         }
      }
   }
   else if(attribute == "player_fall_speed")
   {
      p = parse_all_whitespace(p);
      p = parse_float(p);

      player->fall_speed = p.fToken;
   }
   else
   {
      std::cout << "UNRECOGNIZED ATTRIBUTE AT SCENE DESCRIPTION FILE ('" << path << "') LINE NUMBER " << line_count << "\n";
      assert(false);
   }
}


void parse_and_load_entity(Parse p, ifstream* reader, int& line_count, std::string path)
{
   std::string line;
   bool is_collision_parsed = false;

   Entity* new_entity = new Entity();
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
            new_entity->position = glm::vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      }
      else if(property == "rotation")
      {
            p = parse_float_vector(p);
            new_entity->rotation = glm::vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
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
            new_entity->scale = glm::vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
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
            new_entity->shader = &find->second;
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
         {
            new_entity->mesh = *find->second;
         }
         else
         {
            std::cout << "MESH DATA FOR MESH '" << model_name << "' NOT FOUND WHILE LOADING SCENE DESCRIPTION FILE \n"; 
            assert(false);
         }   
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
         else if(texture_name == "")
         {
            std::cout << "TEXTURE NAME MISSING FOR ENTITY '" << new_entity->name << "' :: FATAL \n"; 
            assert(false);
         }
         else if(texture_type == "")
         {
            std::cout << "TEXTURE TYPE MISSING FOR TEXTURE '" << texture_name << "' AT ENTITY '"<< new_entity->name << "' :: FATAL \n"; 
            assert(false);
         }
         else if(texture_filename == "")
         {
            std::cout << "TEXTURE FILENAME MISSING FOR TEXTURE '"<<texture_name <<"' AT ENTITY '"<<new_entity->name<<"' :: FATAL \n"; 
            assert(false);
         }
         else
         {
            unsigned int texture_id = load_texture_from_file(texture_filename, TEXTURES_PATH);

            if(texture_id == 0)
            {
               std::cout << "TEXTURE '" <<texture_name<< "' COULD NOT BE LOADED WHILE LOADING SCENE DESCRIPTION FILE :: FATAL \n"; 
               assert(false);
            }

            std::string texture_type_def;
            if(texture_type == "diffuse")
            {
               texture_type_def = "texture_diffuse";
            } 
            else if(texture_type == "normal")
            {
               texture_type_def = "texture_normal";
            }
            else
            {
               std::cout<<"TEXTURE '"<<texture_name<<"' HAS UNKNOWN TEXTURE TYPE. ERROR WHILE LOADING SCENE DESCRIPTION FILE :: FATAL \n"; 
               assert(false);
            }

            Texture new_texture{
               texture_id,
               texture_type_def,
               texture_filename
            };

            Texture_Catalogue.insert({texture_name, new_texture});
            new_entity->textures.push_back(new_texture);
         }   
      }
      else if(property == "collision")
      {
         is_collision_parsed = true;       // check so we make sure scale is defined before collision (we use it to define the aabb lengths)

         std::string collision_type;
         p = parse_all_whitespace(p);
         p = parse_token(p);
         collision_type = p.string_buffer;

         if(collision_type == "aabb")
         {
            // CREATES AXIS ALIGNED BOUNDING BOX
            auto cgab = new CollisionGeometryAlignedBox;
            cgab->length_x = new_entity->scale.x;
            cgab->length_y = new_entity->scale.y; 
            cgab->length_z = new_entity->scale.z; 
            
            new_entity->collision_geometry_ptr = cgab;
            new_entity->collision_geometry_type = COLLISION_ALIGNED_BOX;
         }
         else
         {
            std::cout << "UNRECOGNIZED COLLISION TYPE '" << collision_type << "' AT SCENE DESCRIPTION FILE ('" 
                        << path << "') LINE NUMBER " << line_count << "\n";
            assert(false);
         }
      }
      else
      {
         break;
      }
   }

   G_SCENE_INFO.active_scene->entities.push_back(new_entity);
}