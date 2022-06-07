

Entity* create_player_entity()
{
   auto scale = vec3(1);
   auto entity = Entity_Manager.create_entity(PLAYER_NAME, "capsule", "model", "pink", "capsule", scale);
   return entity;
}

Player* create_player(Entity* player_entity)
{
   auto player = new Player();
   player->entity_ptr   = player_entity;
   return player;
}


void parse_and_load_player_orientation(Parser::Parse p, std::ifstream* reader, int& line_count, std::string path, Player* player)
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
      p = parse_vec3(p);
      auto orientation = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      G_SCENE_INFO.views[FPS_CAM]->Front = orientation;
      player->orientation = orientation;
   }
}

void parse_and_load_player_attribute(Parser::Parse p, std::ifstream* reader, int& line_count, std::string path, Player* player)
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
      p = parse_vec3(p);
      auto position = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      player->entity_ptr->position = position;
      player->checkpoint_pos = position;
      player->height_before_fall = position.y;
   }
   else if(attribute == "player_initial_velocity")
   {
      p = parse_vec3(p);
      player->initial_velocity = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      player->entity_ptr->velocity = player->initial_velocity;
   }
   else if(attribute == "player_state")
   {
      p = parse_all_whitespace(p);
      p = parse_int(p);
      player->initial_player_state = (PlayerState) p.iToken;
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
   }
}

bool load_player_attributes_from_file(string scene_name, Player* player)
{
  std::string path = SCENES_FOLDER_PATH + scene_name + ".txt";
   std::ifstream reader(path);

   if(!reader.is_open())
   {
      std::cout << "WARNING: Loading player attributes from file failed.\n";
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
