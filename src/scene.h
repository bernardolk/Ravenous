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
      Entity temp;
      p = parse_name(p);
      temp.name = p.string_buffer;

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

               temp.position = glm::vec3(x,y,z);
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

               temp.rotation = glm::vec3(theta, phi, omega);
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

               temp.scale = glm::vec3(sx, sy, sz);
         }
         else if(property == "shader")
         {

         }
         else if(property == "texture_diffuse")
         {

         }
         else if(property == "model")
         {

         }
      }
   }
} 