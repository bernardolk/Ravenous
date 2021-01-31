void load_scene_entities_from_file(std::string path)
//std::vector<Entity*> &entity_vector
{
   ifstream reader(path);
   std::string line;
   Parse p = parser_start(&reader, &line);
   
   // parses entity
   p = parse_symbol(p);
   if(p.cToken == '#')
   {
      Entity* new_entity = new Entity();
      p = parse_name(p);
      new_entity->name = p.string_buffer;

      while(parser_nextline(&reader, &line, &p))
      {
         p = parse_token(p);
         const std::string property = p.string_buffer;
         if(property == "position")
         {
               float x, y, z;
               p = parse_all_whitespace(p);
               p = parse_float(p);
               x = p.fToken;

               p = parse_all_whitespace(p);
               p = parse_float(p);
               y = p.fToken;

               p = parse_all_whitespace(p);
               p = parse_float(p);
               z = p.fToken;

               new_entity->position = glm::vec3(x,y,z);
         }
         else if(property == "rotation")
         {
               float theta, phi, omega;
               p = parse_all_whitespace(p);
               p = parse_float(p);
               theta = p.fToken;

               p = parse_all_whitespace(p);
               p = parse_float(p);
               phi = p.fToken;

               p = parse_all_whitespace(p);
               p = parse_float(p);
               omega = p.fToken;

               new_entity->rotation = glm::vec3(theta, phi, omega);
         }
         else if(property == "scale")
         {
               float sx, sy, sz;
               p = parse_all_whitespace(p);
               p = parse_float(p);
               sx = p.fToken;

               p = parse_all_whitespace(p);
               p = parse_float(p);
               sy = p.fToken;

               p = parse_all_whitespace(p);
               p = parse_float(p);
               sz = p.fToken;

               new_entity->scale = glm::vec3(sx, sy, sz);
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
         else if(property == "model")
         {
            std::string model_name;
            p = parse_all_whitespace(p);
            p = parse_name(p);
            model_name = p.string_buffer;

            auto find = Model_Catalogue.find(model_name);
            if(find != Model_Catalogue.end())
            {
               new_entity->model = find->second;
            }
            else
            {
               std::cout << "MODEL '" << model_name << "' NOT FOUND WHILE LOADING SCENE DESCRIPTION FILE \n"; 
               assert(false);
            }   
         }
      }

      G_SCENE_INFO.active_scene->entities.push_back(new_entity);
   }
} 