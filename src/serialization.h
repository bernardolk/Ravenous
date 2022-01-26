struct DeferredEntityRelationBuffer {
   static const int size = 64;
   int count = 0;
   Entity* from[size];
   u64 to[size];
   std::string context[size];
};

// Prototypes
bool load_scene_from_file(std::string scene_name, WorldStruct* world);
Entity* parse_and_load_entity(
   Parser::Parse p, ifstream* reader, int& line_count, std::string path, DeferredEntityRelationBuffer* entity_relations
);
void parse_and_load_player_attribute(Parser::Parse p, ifstream* reader, int& line_count, std::string path, Player* player);
void setup_scene_boilerplate_stuff();
bool save_player_position_to_file(string scene_name, Player* player);
bool save_scene_to_file(string scene_name, Player* player, bool do_copy);
void parse_and_load_light_source(Parser::Parse p, ifstream* reader, int& line_count, string path);
void parse_and_load_camera_settings(Parser::Parse p, ifstream* reader, int& line_count, std::string path);
void parse_and_load_player_orientation(Parser::Parse p, ifstream* reader, int& line_count, std::string path, Player* player);
bool load_player_attributes_from_file();
bool check_if_scene_exists();
Entity* create_player_entity();
Player* create_player(Entity* player_entity);
ProgramConfig load_configs();
bool save_configs_to_file();

#include <iomanip>


u64 Max_Entity_Id = 0;


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
   Entity_Manager.set_interactables_registry(&G_SCENE_INFO.active_scene->interactables);

   // creates player
   auto player_entity = create_player_entity();
   auto player = create_player(player_entity);
   G_SCENE_INFO.player = player;

   // creates deferred load buffer for associating entities after loading them all
   auto entity_relations = DeferredEntityRelationBuffer();

   // starts reading
   std::string line;
   Parser::Parse p;
   int line_count = 0;

   // parses header
   parser_nextline(&reader, &line, &p);
   line_count++;

   p = parse_token(p);
   if(!p.hasToken)
      Quit_fatal("Scene '" + scene_name + "' didn't start with NEXT_ENTITY_ID token.");
   
   std::string next_entity_id_token = p.string_buffer;
   if(next_entity_id_token != "NEXT_ENTITY_ID")
      Quit_fatal("Scene '" + scene_name + "' didn't start with NEXT_ENTITY_ID token.");

   p = parse_whitespace(p);
   p = parse_symbol(p);
   if(!p.hasToken || p.cToken != '=')
      Quit_fatal("Missing '=' after NEXT_ENTITY_ID.");

   p = parse_whitespace(p);
   p = parse_u64(p);

   // ENTITY IDs related code
   bool recompute_next_entity_id = false;
   if(!p.hasToken)
      recompute_next_entity_id = true;
   else
      Entity_Manager.next_entity_id = p.u64Token;

   // parses entities
   while(parser_nextline(&reader, &line, &p))
   {
      line_count++;

      p = parse_symbol(p);
      if(p.cToken == '#')
      {
         Entity* new_entity = parse_and_load_entity(p, &reader, line_count, path, &entity_relations);

         // set up collider
         new_entity->collider       = *new_entity->collision_mesh;
         new_entity->collider.name  = new_entity->name + "-collider";
         new_entity->collider.setup_gl_data();

         // puts entity into entities list and update geometric properties
         Entity_Manager.register_in_world_and_scene(new_entity);
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
         parse_and_load_player_orientation(p, &reader, line_count, path, player);
      }
   }

   // connects entities using deferred load buffer
   For(entity_relations.count)
   {
      Entity* from         = entity_relations.from[i];
      std::string context  = entity_relations.context[i];
      Entity* to_entity;

      bool found_to_entity = false;
      Forj(scene->entities.size())
      {  
         Entity* entity = scene->entities[j];
         if(entity->id == entity_relations.to[i])
         {
            to_entity = entity;
            found_to_entity = true;
            break;
         }
      }

      if(!found_to_entity)
      {
         Quit_fatal("Something weird happened. We tried to deferred load the relationship of '"
            + context + "' between (from) '" + from->name + "' and an entity with id '" 
            + to_string(entity_relations.to[i]) + "' but we couldn't find that entity inside the scene list.");
      }

      //@todo: BAD! This requires coordination of literal strings between parts of the module. Need an enum for more clarity.
      //@todo: ALSO, what about introspection/reflection? ... 
      if(context == "timer_target")
      {
         from->timer_target = to_entity;
      }
   }


   // If misisng NEXT_ENTITY_ID in scene header, recompute from collected Ids (If no entity has an ID yet, this will be 1)
   if(recompute_next_entity_id)
   {
      Entity_Manager.next_entity_id = Max_Entity_Id + 1;
   }

   // assign IDs to entities missing them starting from max current id
   For(scene->entities.size())
   {
      auto entity = scene->entities[i];
      if(entity->name != PLAYER_NAME && entity->id == -1)
      {
         entity->id = Entity_Manager.next_entity_id++;
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

   writer << "NEXT_ENTITY_ID = " << Entity_Manager.next_entity_id << "\n";

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


   // @todo WE SHOULD SORT ENTITIES BY IDS BEFORE WRITING! (to minimize git conflicts in the future.)
   // write scene data (for each entity)
   Entity **entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;
      if(entity->name == "Player")
         continue;

      writer << "\n#" << entity->name << "\n";
      writer << "id " << entity->id << "\n";
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
      writer << "shader " << entity->shader->name;

      // shader: If entity is using tiled texture fragment shader, also writes number of tiles since we can change it through the editor
      if(entity->flags & EntityFlags_RenderTiledTexture)
         For(6) {
            writer << " " << entity->uv_tile_wrap[i];
         }

      writer << "\n";

      int textures =  entity->textures.size();
      For(textures)
      {
         Texture texture = entity->textures[i];
         if(texture.type == "texture_diffuse")
            writer << "texture " << texture.name << "\n";
      }

      if(entity->flags & EntityFlags_RenderWireframe)
         writer << "hidden\n";

      switch(entity->type)
      {
         case EntityType_Static:
         {
            writer << "type static\n";
            break;
         }
         case EntityType_Checkpoint:
         {
            writer << "type checkpoint\n";
            writer << "trigger " 
               << entity->trigger_scale.x << " "
               << entity->trigger_scale.y << " "
               << entity->trigger_scale.z << "\n";
            break;
         }
         case EntityType_Timed:
         {
            writer << "type timed\n";
            writer << "trigger " 
               << entity->trigger_scale.x << " "
               << entity->trigger_scale.y << " "
               << entity->trigger_scale.z << "\n";
            if(entity->timer_target != nullptr)
               writer << "timer_target " << entity->timer_target->id << "\n";
            break;
         }
      }

      if(entity->slidable)
      {
         writer << "slidable \n";
      }
   }

   writer.close();

   if(do_copy)
      log(LOG_INFO, "Scene copy saved succesfully as '" + scene_name + ".txt'");
   else if(was_renamed)
   {
      log(LOG_INFO, "Scene saved succesfully as '" + scene_name + ".txt' (now editing it)");
      G_SCENE_INFO.scene_name = scene_name;
   }
   else
      log(LOG_INFO, "Scene saved succesfully.");

   return true;
}


void parse_and_load_camera_settings(Parser::Parse p, ifstream* reader, int& line_count, std::string path)
{
      p = parse_all_whitespace(p);
      p = parse_vec3(p);
      G_SCENE_INFO.camera->Position = vec3{p.vec3[0], p.vec3[1], p.vec3[2]};

      p = parse_all_whitespace(p);
      p = parse_vec3(p);
      camera_look_at(G_SCENE_INFO.camera, vec3{p.vec3[0], p.vec3[1], p.vec3[2]}, false);
}

void parse_and_load_player_orientation(Parser::Parse p, ifstream* reader, int& line_count, std::string path, Player* player)
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


Entity* parse_and_load_entity(
   Parser::Parse p, ifstream* reader, int& line_count, std::string path, DeferredEntityRelationBuffer* entity_relations
){
   std::string line;

   auto new_entity = Entity_Manager.create_entity();
   p = parse_name(p);
   new_entity->name = p.string_buffer;

   while(parser_nextline(reader, &line, &p))
   {
      line_count ++;
      p = parse_token(p);
      const std::string property = p.string_buffer;

      if(property == "id")
      {
         p = parse_all_whitespace(p);
         p = parse_u64(p);
         new_entity->id = p.u64Token;

         if(Max_Entity_Id < p.u64Token)
            Max_Entity_Id = p.u64Token;
      }

      else if(property == "position")
      {
         p = parse_vec3(p);
         new_entity->position = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      }

      else if(property == "rotation")
      {
         p = parse_vec3(p);
         new_entity->rotation = vec3(p.vec3[0],p.vec3[1],p.vec3[2]);
      }

      else if(property == "scale")
      {
         p = parse_vec3(p);
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
         p = parse_token(p);
         shader_name = p.string_buffer;

         auto find = Shader_Catalogue.find(shader_name);
         if(find != Shader_Catalogue.end())
         {
            if(shader_name == "tiledTextureModel")
            {
               new_entity->flags |= EntityFlags_RenderTiledTexture;
               For(6)
               {
                  p = parse_all_whitespace(p);
                  p = parse_int(p);
                  if(!p.hasToken)
                     Quit_fatal("Scene description contain an entity with box tiled shader without full tile quantity description.");

                  new_entity->uv_tile_wrap[i] = p.iToken;
               }
            }
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
         // @TODO when we get REAL about this, collision mesh should be a separate mesh (of course).
         new_entity->collision_mesh = new_entity->mesh;
      }
      
      else if(property == "texture")
      {
         // @TODO def_2 is unnecessary now. After scene files don't contain it anymore, lets drop support.
         std::string texture_def_1, texture_def_2;
         p = parse_all_whitespace(p);
         p = parse_token(p);
         texture_def_1 = p.string_buffer;

         p = parse_all_whitespace(p);
         p = parse_token(p);
         texture_def_2 = p.string_buffer;

         // > texture definition error handling
         // >> check for missing info
         if(texture_def_1 == "")
         {
            std::cout << "Fatal: Texture for entity '" << new_entity->name << "' is missing name. \n"; 
            assert(false);
         }
         
         // @TODO: for backwards compability
         string texture_name = texture_def_1;
         if(texture_def_2 != "")
            texture_name = texture_def_2;

         // fetches texture in catalogue
         auto texture = Texture_Catalogue.find(texture_name);
         if(texture == Texture_Catalogue.end())
         {
            std::cout<<"Fatal: '"<< texture_name <<"' was not found (not pre-loaded) inside Texture Catalogue \n"; 
            assert(false);
         }

         new_entity->textures.push_back(texture->second);

         // fetches texture normal in catalogue, if any
         auto normal = Texture_Catalogue.find(texture_name + "_normal");
         if(normal != Texture_Catalogue.end())
         {
            new_entity->textures.push_back(normal->second);
         }
      }

      else if(property == "hidden")
      {
         new_entity->flags |= EntityFlags_HiddenEntity;
      }

      else if(property == "type")
      {
         p = parse_all_whitespace(p);
         p = parse_token(p);
         std::string entity_type = p.string_buffer;
         if(entity_type == "static")
            Entity_Manager.set_type(new_entity, EntityType_Static);
         else if(entity_type == "checkpoint")
            Entity_Manager.set_type(new_entity, EntityType_Checkpoint);
         else if(entity_type == "timed")
            Entity_Manager.set_type(new_entity, EntityType_Timed);
         else
            Quit_fatal("Entity type '" + entity_type + "' not identified.");
      }

      else if(property == "timer_target")
      {
         p = parse_all_whitespace(p);
         p = parse_u64(p);
         auto timer_target_id = p.u64Token;

         int i                         = entity_relations->count;
         entity_relations->to[i]       = timer_target_id;
         entity_relations->from[i]     = new_entity;
         entity_relations->context[i]  = "timer_target";
         entity_relations->count++;
      }

      else if(property == "trigger")
      {
         p = parse_vec3(p);
         new_entity->trigger_scale = vec3{p.vec3[0], p.vec3[1], p.vec3[2]};
      }

      else if(property == "slidable")
      {
         new_entity->slidable = true;
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

bool save_player_position_to_file(string scene_name, Player* player)
{
   // NOT IMPLEMENTED
   return false;
}


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