
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
            p = parse_vec3(p);
            point_light.position = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "diffuse")
         {
            p = parse_vec3(p);
            point_light.diffuse = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "specular")
         {
            p = parse_vec3(p);
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
            p = parse_vec3(p);
            spotlight.position = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "direction")
         {
            p = parse_vec3(p);
            spotlight.direction = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "diffuse")
         {
            p = parse_vec3(p);
            spotlight.diffuse = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "specular")
         {
            p = parse_vec3(p);
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
            p = parse_vec3(p);
            light.direction = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "diffuse")
         {
            p = parse_vec3(p);
            light.diffuse = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else if(property == "specular")
         {
            p = parse_vec3(p);
            light.specular = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
         }
         else break;
      }

      G_SCENE_INFO.active_scene->directionalLights.push_back(light);     
   }
}