void parse_and_load_camera_settings(Parser::Parse p, ifstream* reader, int& line_count, std::string path)
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