
bool load_scene_from_file(std::string scene_name);
void parse_and_load_entity(Parser::Parse p, ifstream* reader, int& line_count, std::string path);
void parse_and_load_attribute(Parser::Parse p, ifstream* reader, int& line_count, std::string path, Player* player);
void setup_scene_boilerplate_stuff();
void save_player_position_to_file(string scene_name);
bool save_scene_to_file(string scene_name, Player* player, bool do_copy);

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

   // write player attributes to file
   writer << "@player_position = " 
               << player->entity_ptr->position.x << " " 
               << player->entity_ptr->position.y << " "
               << player->entity_ptr->position.z << "\n";
   writer << "@player_initial_velocity = "
               << player->initial_velocity.x << " " 
               << player->initial_velocity.y << " "
               << player->initial_velocity.z << "\n";
   writer << "@player_state = " << player->initial_player_state << "\n"; 
   writer << "@player_fall_acceleration = " << player->fall_acceleration << "\n";  

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
      writer << "mesh " << entity->mesh.name << "\n";
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

      string collision_type;
      if(entity->collision_geometry_type == COLLISION_ALIGNED_BOX)
         collision_type = "aabb";
      else if(entity->collision_geometry_type == COLLISION_ALIGNED_SLOPE)
         collision_type = "slope";
      else
         assert(false);

      writer << "collision " << collision_type << "\n";
   }

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

bool load_scene_from_file(std::string scene_name)
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

   // creates player and some other entities
   setup_scene_boilerplate_stuff();

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
         parse_and_load_entity(p, &reader, line_count, path);
      }
      else if(p.cToken == '@')
      {
         parse_and_load_attribute(p, &reader, line_count, path, G_SCENE_INFO.player);
      }
   }
   
   G_SCENE_INFO.scene_name = scene_name;
   return true;
} 

void parse_and_load_attribute(Parser::Parse p, ifstream* reader, int& line_count, std::string path, Player* player)
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
      player->entity_ptr->position = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
   }
   else if(attribute == "player_initial_velocity")
   {
      p = parse_float_vector(p);
      player->initial_velocity = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      player->entity_ptr->velocity = player->initial_velocity;
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


void parse_and_load_entity(Parser::Parse p, ifstream* reader, int& line_count, std::string path)
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
            if(!(texture_type == "texture_diffuse" || texture_type == "texture_normal"))
            {
               std::cout<<"TEXTURE '"<<texture_name<<"' HAS UNKNOWN TEXTURE TYPE " <<texture_type <<
                  ". ERROR WHILE LOADING SCENE DESCRIPTION FILE :: FATAL \n"; 
               assert(false);
            }

            Texture new_texture{
               texture_id,
               texture_type,
               texture_filename,
               texture_name
            };

            Texture_Catalogue.insert({texture_name, new_texture});
            new_entity->textures.push_back(new_texture);
         }   
      }
      else if(property == "collision")
      {
         // check so we make sure scale is defined before collision (we use it to define the aabb lengths)
         is_collision_parsed = true;

         std::string collision_type;
         p = parse_all_whitespace(p);
         p = parse_token(p);
         collision_type = p.string_buffer;

         if(collision_type == "aabb")
         {
            new_entity->collision_geometry.aabb = CollisionGeometryAlignedBox {
               new_entity->scale.x,
               new_entity->scale.y,
               new_entity->scale.z
            };

            new_entity->collision_geometry_type = COLLISION_ALIGNED_BOX;
         }
         else if(collision_type == "slope")
         {
            auto& slope    = new_entity->collision_geometry.slope;
            slope.length   = new_entity->scale.x;
            slope.width    = new_entity->scale.z;
            slope.height   = new_entity->scale.y;

            new_entity->set_slope_properties();

            assert((int)new_entity->rotation.x % 90 == 0);
            assert((int)new_entity->rotation.y % 90 == 0);
            assert((int)new_entity->rotation.z % 90 == 0);

            assert(new_entity->scale.x > 0);
            assert(new_entity->scale.y > 0); 
            assert(new_entity->scale.z > 0);

            new_entity->collision_geometry_type = COLLISION_ALIGNED_SLOPE;
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

void setup_scene_boilerplate_stuff()
{
   // CREATE SCENE 
   auto demo_scene = new Scene();

   // PLAYER ENTITY SETUP

   // CYLINDER
   unsigned int pink_texture = load_texture_from_file("pink.jpg", TEXTURES_PATH);
    auto cylinder_texture = new Texture
    {
      pink_texture,
      "texture_diffuse",
      "whatever"
   };
   auto find1 = Shader_Catalogue.find("model");
   auto model_shader = find1->second;

   auto find2 = Geometry_Catalogue.find("quad");
   auto quad_mesh = find2->second;

   auto find_cylinder = Geometry_Catalogue.find("player_cylinder");
   auto cylinder_mesh = find_cylinder->second;

   auto cylinder = new Entity();
   cylinder->name             = "Player";
   cylinder->index            = G_ENTITY_INFO.entity_counter;
   cylinder->id               = ++G_ENTITY_INFO.entity_counter;
   cylinder->shader           = model_shader;
   cylinder->position         = vec3(0,1,1);
   cylinder->textures         = std::vector<Texture>{*cylinder_texture};
   cylinder->mesh             = *cylinder_mesh;
   // player collision geometry
   cylinder->collision_geometry_type = COLLISION_ALIGNED_CYLINDER;
   auto cgac = new CollisionGeometryAlignedCylinder { CYLINDER_HALF_HEIGHT, CYLINDER_RADIUS };
   cylinder->collision_geometry.cylinder = *cgac;

   demo_scene->entities.push_back(cylinder);

   // lightsource
   auto l1 = new PointLight();
   l1->id                     = 1;
   l1->position               = vec3(0.5, 3.5, 0.5);
   l1->diffuse                = vec3(1.0, 1.0, 1.0);
   l1->ambient                = vec3(1.0,1.0,1.0);
   l1->intensity_linear       = 0.4f;
   l1->intensity_quadratic    = 0.04f;
   demo_scene->pointLights.push_back(*l1);

   auto l2 = new PointLight();
   l2->id                     = 2;
   l2->position               = vec3(-8, 10, 1);
   l2->diffuse                = vec3(1.0, 1.0, 1.0);
   l2->ambient                = vec3(1.0,1.0,1.0);
   l2->intensity_linear       = 0.4f;
   l2->intensity_quadratic    = 0.04f;
   demo_scene->pointLights.push_back(*l2);

   G_SCENE_INFO.active_scene = demo_scene;

   // create player
   auto player = new Player();
   player->entity_ptr   = cylinder;
   player->half_height  = CYLINDER_HALF_HEIGHT;
   player->radius       = CYLINDER_RADIUS;

   G_SCENE_INFO.player = player;
}

void save_player_position_to_file(string scene_name)
{
   // NOT IMPLEMENTED
}
