#pragma once

const std::string  SrLoadEntity_TypeNotSetErrorMsg = "Need to load entity type before loading type-specific data.";

Entity* parse_and_load_entity(
   Parser::Parse p, 
   std::ifstream* reader, 
   int& line_count, 
   std::string path, 
   DeferredEntityRelationBuffer* entity_relations)
{
   std::string line;

   bool type_set = false;

   auto new_entity = Entity_Manager.create_entity({});
   p = parse_name(p);
   new_entity->name = p.string_buffer;

   while(parser_nextline(reader, &line, &p))
   {
      line_count ++;
      p = parse_token(p);
      const std::string  property = p.string_buffer;

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
         
         auto find_mesh = Geometry_Catalogue.find(model_name);
         if(find_mesh != Geometry_Catalogue.end())
            new_entity->mesh = find_mesh->second;
         else
            new_entity->mesh = load_wavefront_obj_as_mesh(MODELS_PATH, model_name);
      
         // @TODO: For now collision mesh is loaded from the same model as regular mesh.
         auto find_c_mesh = Collision_Geometry_Catalogue.find(model_name);
         if(find_c_mesh != Collision_Geometry_Catalogue.end())
            new_entity->collision_mesh = find_c_mesh->second;
         else
            new_entity->collision_mesh = load_wavefront_obj_as_collision_mesh(MODELS_PATH, model_name);
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
        std::string texture_name = texture_def_1;
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

         if (entity_type == SrStr_EntityType_Static)
            Entity_Manager.set_type(new_entity, EntityType_Static);

         else if(entity_type == SrStr_EntityType_Checkpoint)
            Entity_Manager.set_type(new_entity, EntityType_Checkpoint);

         else if(entity_type == SrStr_EntityType_TimerTrigger)
            Entity_Manager.set_type(new_entity, EntityType_TimerTrigger);

         else if(entity_type == SrStr_EntityType_TimerTarget)
            Entity_Manager.set_type(new_entity, EntityType_TimerTarget);

         else if(entity_type == SrStr_EntityType_TimerMarking)
            Entity_Manager.set_type(new_entity, EntityType_TimerMarking);

         else
            Quit_fatal("Entity type '" + entity_type + "' not identified.");

         type_set = true;
      }

      // ---------------------------------
      // > entity type related properties
      // ---------------------------------

      else if(property == "timer_target")
      {
         if(!type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p = parse_all_whitespace(p);
         p = parse_u64(p);
         auto timer_target_id = p.u64Token;

         int i                                     = entity_relations->count;
         entity_relations->deferred_entity_ids[i]  = timer_target_id;
         entity_relations->entities[i]             = new_entity;
         entity_relations->relations[i]            = SrEntityRelation_TimerTarget;
         entity_relations->count++;
      }

      else if(property == "timer_duration")
      {
         if(!type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p = parse_all_whitespace(p);
         p = parse_float(p);
         new_entity->timer_trigger_data.timer_duration = p.fToken;
      }

      else if(property == "timer_target_type")
      {
         if(!type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p = parse_all_whitespace(p);
         p = parse_uint(p);
         auto tt_type = (EntityTimerTargetType) p.uiToken;
         new_entity->timer_target_data.timer_target_type = tt_type;
      }

      else if(property == "timer_start_animation")
      {
         if(!type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p = parse_all_whitespace(p);
         p = parse_uint(p);

         auto tsa = p.uiToken;
         new_entity->timer_target_data.timer_start_animation = tsa;
      }

      else if(property == "timer_stop_animation")
      {
         if(!type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p = parse_all_whitespace(p);
         p = parse_uint(p);

         auto tsa = p.uiToken;
         new_entity->timer_target_data.timer_stop_animation = tsa;
      }

      else if(property == "timer_marking")
      {
         if(!type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p = parse_all_whitespace(p);
         p = parse_uint(p);
         auto marking_id = p.uiToken;

         p = parse_all_whitespace(p);
         p = parse_uint(p);
         auto marking_time_checkpoint = p.uiToken;

         int i                                     = entity_relations->count;
         entity_relations->deferred_entity_ids[i]  = marking_id;
         entity_relations->entities[i]             = new_entity;
         entity_relations->relations[i]            = SrEntityRelation_TimerMarking;
         entity_relations->aux_uint_buffer[i]      = marking_time_checkpoint;
         entity_relations->count++;
      }

      else if(property == "timer_marking_color_on")
      {
         p = parse_vec3(p);
         new_entity->timer_marking_data.color_on = p.get_vec3_val();
      }

      else if(property == "timer_marking_color_off")
      {
         p = parse_vec3(p);
         new_entity->timer_marking_data.color_off = p.get_vec3_val();
      }

      // ---------------------------------

      else if(property == "trigger")
      {
         p = parse_vec3(p);
         new_entity->trigger_scale = p.get_vec3_val();
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