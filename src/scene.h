
bool load_scene_from_file(std::string scene_name);
void parse_and_load_entity(Parse p, ifstream* reader, int& line_count, std::string path);
void parse_and_load_attribute(Parse p, ifstream* reader, int& line_count, std::string path, Player* player);
void setup_scene_boilerplate_stuff();
void save_player_position_to_file(string scene_name);



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
         parse_and_load_attribute(p, &reader, line_count, path, G_SCENE_INFO.player);
      }
   }
   
   G_SCENE_INFO.scene_name = scene_name;
   return true;
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
      player->entity_ptr->position = glm::vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
   }
   else if(attribute == "player_initial_velocity")
   {
      p = parse_float_vector(p);
      player->entity_ptr->velocity = glm::vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
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
         // check so we make sure scale is defined before collision (we use it to define the aabb lengths)
         is_collision_parsed = true;

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
         else if(collision_type == "slope")
         {
            auto slope_collision = new CollisionGeometrySlope;
            slope_collision->slope_width =  new_entity->scale.z;
            slope_collision->slope_height = new_entity->scale.y;
            slope_collision->slope_length = new_entity->scale.x;

            float inclination = slope_collision->slope_height / slope_collision->slope_length;
            float slope_angle = atan(inclination);
            float complementary = 180.0f - (slope_angle + 90.0f); 

            // slope geometry is defined as default (rotation = 0) being going down along +x
            // here we set the tangent vector to the slope, so the player falls along it when sliding
            auto slope_direction = glm::vec3(0, -1 * sin(slope_angle), 0);
            auto slope_normal = glm::vec3(0, sin(complementary), 0);
            switch((int) new_entity->rotation.y)
            {
               case 0:
               {
                  slope_direction.x = cos(slope_angle);
                  slope_normal.x = cos(complementary);
                  break;
               }
               case 90:
               {
                  slope_direction.z = -1 * cos(slope_angle);
                  slope_normal.z = -1 * cos(complementary);
                  break;
               }
               case 180:
               {
                  slope_direction.x = -1 * cos(slope_angle);
                  slope_normal.x = -1 * cos(complementary);
                  break;
               }
               case 270:
               {
                  slope_direction.z = cos(slope_angle);
                  slope_normal.z = cos(complementary);
                  break;
               }
            }
            slope_collision->tangent = slope_direction;
            slope_collision->normal = slope_normal;
            slope_collision->inclination = inclination;

            assert((int)new_entity->rotation.x % 90 == 0);
            assert((int)new_entity->rotation.y % 90 == 0);
            assert((int)new_entity->rotation.z % 90 == 0);

            assert(new_entity->scale.x > 0);
            assert(new_entity->scale.y > 0); 
            assert(new_entity->scale.z > 0);

            new_entity->collision_geometry_ptr = slope_collision;
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
   cylinder->position         = glm::vec3(0,1,1);
   cylinder->textures         = std::vector<Texture>{*cylinder_texture};
   cylinder->mesh             = *cylinder_mesh;
   // player collision geometry
   cylinder->collision_geometry_type = COLLISION_ALIGNED_CYLINDER;
   auto cgac = new CollisionGeometryAlignedCylinder { CYLINDER_HALF_HEIGHT, CYLINDER_RADIUS };
   cylinder->collision_geometry_ptr = cgac;

   demo_scene->entities.push_back(cylinder);

   // lightsource
   auto l1 = new PointLight();
   l1->id                     = 1;
   l1->position               = glm::vec3(0.5, 3.5, 0.5);
   l1->diffuse                = glm::vec3(1.0, 1.0, 1.0);
   l1->ambient                = glm::vec3(1.0,1.0,1.0);
   l1->intensity_linear       = 0.4f;
   l1->intensity_quadratic    = 0.04f;
   demo_scene->pointLights.push_back(*l1);

   auto l2 = new PointLight();
   l2->id                     = 2;
   l2->position               = glm::vec3(-8, 10, 1);
   l2->diffuse                = glm::vec3(1.0, 1.0, 1.0);
   l2->ambient                = glm::vec3(1.0,1.0,1.0);
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
