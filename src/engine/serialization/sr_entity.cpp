#include <string>
#include <iostream>
#include <engine/core/rvn_types.h>
#include <engine/logging.h>
#include <engine/serialization/sr_entity.h>
#include <engine/collision/collision_mesh.h>
#include "engine/entity.h"
#include <engine/entity_manager.h>
#include <engine/serialization/sr_common.h>
#include "rvn_macros.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_entity.h"

const std::string  SrLoadEntity_TypeNotSetErrorMsg = "Need to load entity type before loading type-specific data.";

void EntitySerializer::parse(Parser& parser)
{
   auto new_entity   = manager.create_entity({});
   bool is_type_set  = false;
   
   auto& p = parser;
   p.parse_name();
   new_entity->name = get_parsed<std::string>(parser);

   while(parser.next_line())
   {
      p.parse_token();
      const auto property = get_parsed<std::string>(parser);

      if(property == "id")
      {
         p.parse_all_whitespace();
         p.parse_u64();
         u64 id = get_parsed<u64>(parser);
         new_entity->id = id;

         if(manager.next_entity_id < id)
            manager.next_entity_id = id;
      }

      else if(property == "position")
      {
         p.parse_vec3();
         new_entity->position = get_parsed<glm::vec3>(parser);
      }

      else if(property == "rotation")
      {
         p.parse_vec3();
         new_entity->rotation = get_parsed<glm::vec3>(parser);
      }

      else if(property == "scale")
      {
         p.parse_vec3();
         const auto s = get_parsed<glm::vec3>(parser);
         
         if(s.x < 0 || s.y < 0 || s.z < 0)
         {
            std::cout << "FATAL: ENTITY SCALE PROPERTY CANNOT BE NEGATIVE. AT '" << parser.filepath
                        << "' LINE NUMBER " << parser.line_count << "\n";
         }
         new_entity->scale = s;
      }

      else if(property == "shader")
      {
         p.parse_all_whitespace();
         p.parse_token();
         const auto shader_name = get_parsed<std::string>(parser);

         auto find = Shader_Catalogue.find(shader_name);
         if(find != Shader_Catalogue.end())
         {
            if(shader_name == "tiledTextureModel")
            {
               new_entity->flags |= EntityFlags_RenderTiledTexture;
               For(6)
               {
                  p.parse_all_whitespace();
                  p.parse_int();
                  if(!p.has_token())
                     Quit_fatal("Scene description contain an entity with box tiled shader without full tile quantity description.");

                  new_entity->uv_tile_wrap[i] = get_parsed<int>(parser);
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
         p.parse_all_whitespace();
         p.parse_token();
         const auto model_name = get_parsed<std::string>(parser);
         
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

         new_entity->collider = *new_entity->collision_mesh;
      }
      
      else if(property == "texture")
      {
         // @TODO def_2 is unnecessary now. After scene files don't contain it anymore, lets drop support.
         std::string texture_def_1, texture_def_2;
         p.parse_all_whitespace();
         p.parse_token();
         texture_def_1 = get_parsed<std::string>(parser);

         p.parse_all_whitespace();
         p.parse_token();
         texture_def_2 = get_parsed<std::string>(parser);

         // > texture definition error handling
         // >> check for missing info
         if(!texture_def_1.empty())
         {
            std::cout << "Fatal: Texture for entity '" << new_entity->name << "' is missing name. \n"; 
            assert(false);
         }
         
         // @TODO: for backwards compability
        std::string texture_name = texture_def_1;
         if(!texture_def_2.empty())
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
         p.parse_all_whitespace();
         p.parse_token();
         std::string entity_type = get_parsed<std::string>(parser);

         if (entity_type == SrStr_EntityType_Static)
            manager.set_type(new_entity, EntityType_Static);

         else if(entity_type == SrStr_EntityType_Checkpoint)
            manager.set_type(new_entity, EntityType_Checkpoint);

         else if(entity_type == SrStr_EntityType_TimerTrigger)
            manager.set_type(new_entity, EntityType_TimerTrigger);

         else if(entity_type == SrStr_EntityType_TimerTarget)
            manager.set_type(new_entity, EntityType_TimerTarget);

         else if(entity_type == SrStr_EntityType_TimerMarking)
            manager.set_type(new_entity, EntityType_TimerMarking);

         else
            Quit_fatal("Entity type '" + entity_type + "' not identified.");

         is_type_set = true;
      }

      // ---------------------------------
      // > entity type related properties
      // ---------------------------------

      else if(property == "timer_target")
      {
         if(!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p.parse_all_whitespace();
         p.parse_u64();
         auto timer_target_id = get_parsed<u64>(parser);

         int i = relations.count;
         relations.deferred_entity_ids[i]  = timer_target_id;
         relations.entities[i]             = new_entity;
         relations.relations[i]            = SrEntityRelation_TimerTarget;
         relations.count++;
      }

      else if(property == "timer_duration")
      {
         if(!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p.parse_all_whitespace();
         p.parse_float();
         new_entity->timer_trigger_data.timer_duration = get_parsed<int>(parser);
      }

      else if(property == "timer_target_type")
      {
         if(!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p.parse_all_whitespace();
         p.parse_uint();
         new_entity->timer_target_data.timer_target_type = (EntityTimerTargetType) get_parsed<u32>(parser);
      }

      else if(property == "timer_start_animation")
      {
         if(!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p.parse_all_whitespace();
         p.parse_uint();
         new_entity->timer_target_data.timer_start_animation = get_parsed<u32>(parser);
      }

      else if(property == "timer_stop_animation")
      {
         if(!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p.parse_all_whitespace();
         p.parse_uint();
         new_entity->timer_target_data.timer_stop_animation = get_parsed<u32>(parser);
      }

      else if(property == "timer_marking")
      {
         if(!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

         p.parse_all_whitespace();
         p.parse_uint();
         const auto marking_id = get_parsed<u32>(parser);

         p.parse_all_whitespace();
         p.parse_uint();
         const auto marking_time_checkpoint = get_parsed<u32>(parser);

         int i = relations.count;
         relations.deferred_entity_ids[i]  = marking_id;
         relations.entities[i]             = new_entity;
         relations.relations[i]            = SrEntityRelation_TimerMarking;
         relations.aux_uint_buffer[i]      = marking_time_checkpoint;
         relations.count++;
      }

      else if(property == "timer_marking_color_on")
      {
         p.parse_vec3();
         new_entity->timer_marking_data.color_on = get_parsed<glm::vec3>(parser);
      }

      else if(property == "timer_marking_color_off")
      {
         p.parse_vec3();
         new_entity->timer_marking_data.color_off = get_parsed<glm::vec3>(parser);
      }

      // ---------------------------------

      else if(property == "trigger")
      {
         p.parse_vec3();
         new_entity->trigger_scale = get_parsed<glm::vec3>(parser);
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

   // Register entity in world
   manager.register_in_world_and_scene(new_entity);
}

void EntitySerializer::_clear_buffer()
{
   relations = DeferredEntityRelationBuffer{};
}
