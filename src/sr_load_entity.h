
Entity* parse_and_load_entity(
   Parser::Parse p, 
   ifstream* reader, 
   int& line_count, 
   std::string path, 
   DeferredEntityRelationBuffer* entity_relations)
{
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
         else if(entity_type == "timer_trigger")
            Entity_Manager.set_type(new_entity, EntityType_TimerTrigger);
         else
            Quit_fatal("Entity type '" + entity_type + "' not identified.");
      }

      // ---------------------------------
      // > timer trigger related settings
      // ---------------------------------

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

      else if(property == "timer_duration")
      {
         p = parse_all_whitespace(p);
         p = parse_float(p);
         new_entity->timer_duration = p.fToken;
      }

      // ---------------------------------
      // > timer target related settings
      // ---------------------------------
      
      else if(property == "timer_target_type")
      {
         p = parse_all_whitespace(p);
         p = parse_uint(p);
         auto tt_type = (EntityTimerTargetType) p.uiToken;

         new_entity->is_timer_target = true;
         new_entity->timer_target_type = tt_type;
      }

      // -----------------------------
      // > animation related settings
      // -----------------------------

      else if(property == "timer_start_animation")
      {
         p = parse_all_whitespace(p);
         p = parse_token(p);

         std::string tsa = p.string_buffer;
         new_entity->timer_start_animation = tsa;
      }

      else if(property == "timer_stop_animation")
      {
         p = parse_all_whitespace(p);
         p = parse_token(p);

         std::string tsa = p.string_buffer;
         new_entity->timer_stop_animation = tsa;
      }


      // ---------------------------------

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