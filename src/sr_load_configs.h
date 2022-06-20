void parse_and_load_camera_settings(Parser::Parse p, std::ifstream* reader, int& line_count, std::string path)
{
      p = parse_all_whitespace(p);
      p = parse_vec3(p);
      G_SCENE_INFO.camera->Position = vec3{p.vec3[0], p.vec3[1], p.vec3[2]};

      p = parse_all_whitespace(p);
      p = parse_vec3(p);
      camera_look_at(G_SCENE_INFO.camera, vec3{p.vec3[0], p.vec3[1], p.vec3[2]}, false);
}

ProgramConfig load_configs()
{
   std::ifstream reader(CONFIG_FILE_PATH);

   if(!reader.is_open())
   {
      std::cout << "FATAL: Cant load config file '" + CONFIG_FILE_PATH + "', path NOT FOUND \n";  
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
     std::string attribute = p.string_buffer;

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
         p = parse_vec3(p);
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

float* load_camera_settings(std::string path)
{
   std::ifstream reader(path);
	std::string line;

	static float camera_settings[6];

   // get camera position
   {
	   getline(reader, line);
      const char* cline = line.c_str();
      size_t size = line.size();

      Parser::Parse p { 
         cline, 
         size 
      };

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[0] = p.fToken;

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[1] = p.fToken;

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[2] = p.fToken;
   }
    // get camera direction
   {
	   getline(reader, line);
      const char* cline = line.c_str();
      size_t size = line.size();

      Parser::Parse p { 
         cline, 
         size 
      };

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[3] = p.fToken;

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[4] = p.fToken;

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[5] = p.fToken;
   }

   return &camera_settings[0];
}