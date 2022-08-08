#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <engine/core/rvn_types.h>
#include <rvn_macros.h>
#include <engine/logging.h>
#include <engine/rvn.h>
#include <player.h>
#include <engine/entity.h>
#include "engine/camera.h"
#include <scene.h>
#include <engine/world/world.h>
#include <engine/entity_manager.h>
#include <engine/serialization/sr_entity.h>
#include <engine/serialization/sr_player.h>
#include <engine/serialization/sr_light.h>
#include <engine/serialization/sr_config.h>
#include <engine/serialization/parsing/parser.h>
#include <engine/serialization/sr_world.h>


bool WorldSerializer::load_from_file(const std::string& filename)
{
   const auto path = SCENES_FOLDER_PATH + filename + ".txt";

   // clears the current scene entity data
   world->clear(manager);

   // Gets a new world struct from scratch
   world->init();

   // Either creates new scene or resets current structures
   if(G_SCENE_INFO.active_scene == nullptr)
   {
      G_SCENE_INFO.active_scene  = new Scene();
      G_SCENE_INFO.player        = new Player();
   }
   else
   {
      *G_SCENE_INFO.active_scene = Scene{};
      *G_SCENE_INFO.player       = Player{};
   }

   world->player = G_SCENE_INFO.player;

   G_SCENE_INFO.player->entity_ptr = manager->create_entity({
      .name = PLAYER_NAME,
      .mesh = "capsule",
      .shader = "model",
      .texture = "pink",
      .collision_mesh = "capsule",
      .scale = vec3(1)});

   // creates deferred load buffer for associating entities after loading them all
   auto entity_relations = DeferredEntityRelationBuffer();

   // starts reading
   auto p = Parser{path};

   // parses header
   p.next_line();
   p.parse_token();
   if(!p.has_token())
      Quit_fatal("Scene '" + filename + "' didn't start with NEXT_ENTITY_ID token.")
   
   const auto next_entity_id_token = get_parsed<std::string>(p);
   if(next_entity_id_token != "NEXT_ENTITY_ID")
      Quit_fatal("Scene '" + filename + "' didn't start with NEXT_ENTITY_ID token.")

   p.parse_whitespace();
   p.parse_symbol();
   if(!p.has_token() || get_parsed<char>(p) != '=')
      Quit_fatal("Missing '=' after NEXT_ENTITY_ID.")

   p.parse_whitespace();
   p.parse_u64();

   // ENTITY IDs related code
   bool recompute_next_entity_id = false;
   if(!p.has_token())   recompute_next_entity_id   = true;
   else                 manager->next_entity_id     = get_parsed<u64>(p);

   // -----------------------------------
   //           Parse entities
   // -----------------------------------
   while(p.next_line())
   {
      p.parse_symbol();
      switch(get_parsed<char>(p))
      {
         case '#':
            EntitySerializer::parse(p);
            break;
         
         case '@':
            PlayerSerializer::parse_attribute(p);
            break;
         
         case '$':
            LightSerializer::parse(p);
            break;
         
         case '*':
            ConfigSerializer::parse_camera_settings(p);
            break;
            
         case '&':
            PlayerSerializer::parse_orientation(p);
            break;
            
         default:
            break;
      }
   }

   // -----------------------------------
   //          Post parse steps
   // -----------------------------------
   world->player->update(world, true);
   CL_update_player_world_cells(G_SCENE_INFO.player, world);

   // connects entities using deferred load buffer
   For(entity_relations.count)
   {
      //@TODO: wtf is going on with 2 entity vars being declared here?
      Entity* entity                = entity_relations.entities[i];
      auto relation                 = entity_relations.relations[i];
      auto deferred_entity_id       = entity_relations.deferred_entity_ids[i];
      Entity* deferred_entity       = nullptr;

      Forj(world->entities.size())
      {  
         Entity* entity = world->entities[j];
         if(entity->id == deferred_entity_id)
         {
            deferred_entity = entity;
            break;
         }
      }

      if(deferred_entity == nullptr)
         Quit_fatal("Entity with id '" + std::to_string(deferred_entity_id) + "' not found to stablish a defined entity relationship.")

      switch(relation)
      {
         case SrEntityRelation_TimerTarget:
         {
            entity->timer_trigger_data.timer_target = deferred_entity;
            break;
         }

         case SrEntityRelation_TimerMarking:
         {
            u32 time_checkpoint = entity_relations.aux_uint_buffer[i];
            entity->timer_trigger_data.add_marking(deferred_entity, time_checkpoint);
            break;
         }
      }
   }


   // -----------------------------------
   //         Entity id bookkeeping
   // -----------------------------------

   // If missing NEXT_ENTITY_ID in scene header, recompute from collected Ids (If no entity has an ID yet, this will be 1)
   if(recompute_next_entity_id)
   {
      manager->next_entity_id = Max_Entity_Id + 1;
   }

   // assign IDs to entities missing them starting from max current id
   For(world->entities.size())
   {
      if(auto entity = world->entities[i]; entity->name != PLAYER_NAME && entity->id == -1)
      {
         entity->id = manager->next_entity_id++;
      }
   }

   // clear static relations buffer
   EntitySerializer::_clear_buffer();
   
   world->update_cells_in_use_list();

   G_SCENE_INFO.scene_name = filename;

   // save backup
   // save_scene_to_file("backup", G_SCENE_INFO.player, world, true);

   return true;
}

bool WorldSerializer::save_to_file()
{
   const std::string f;
   return save_to_file(f, false);
}

bool WorldSerializer::save_to_file(const std::string& new_filename, const bool do_copy = false)
{
   std::string filename;
   if(new_filename.empty())
   {
      filename = G_SCENE_INFO.scene_name;

      if(do_copy)
      {
         std::cout << "please provide a name for the copy.\n";
         return false;
      }
   }

   const auto path = SCENES_FOLDER_PATH + filename + ".txt";
   std::ofstream writer(path);
   
   if(!writer.is_open())
   {
      std::cout << "Saving scene failed.\n";
      return false;
   }

   writer << std::fixed << std::setprecision(4);

   writer << "NEXT_ENTITY_ID = " << manager->next_entity_id << "\n";

   // @TODO: Refactor this at some point
   // write camera settings to file
   const auto camera = ConfigSerializer::scene_info->views[0];
   writer << "*" 
      << camera->Position.x << " "
      << camera->Position.y << " "
      << camera->Position.z << "  "
      << camera->Front.x << " "
      << camera->Front.y << " "
      << camera->Front.z << "\n";

   // write player attributes to file
   PlayerSerializer::save(writer);

   // @TODO: Refactor this at some point
   // write player orientation
   const auto fps_cam = G_SCENE_INFO.views[FPS_CAM];
   writer << "&player_orientation = "
      << fps_cam->Front.x << " "
      << fps_cam->Front.y << " "
      << fps_cam->Front.z << "\n"; 
   
   // write lights to file
   for(const auto& light : world->point_lights)
      LightSerializer::save(writer, light);

   for(const auto& light : world->spot_lights)
      LightSerializer::save(writer, light);

   for(const auto& light : world->directional_lights)
      LightSerializer::save(writer, light);
   

   for(const auto& entity : world->entities) 
   {
      if(entity->name == "Player")
         continue;

      EntitySerializer::save(writer, *entity);
   }

   writer.close();

   if(do_copy)
   {
      log(LOG_INFO, "Scene copy saved successfully as '" + filename + ".txt'");
   }
   else if(!new_filename.empty())
   {
      log(LOG_INFO, "Scene saved successfully as '" + filename + ".txt' (now editing it)");
      G_SCENE_INFO.scene_name = filename;
   }
   else
      log(LOG_INFO, "Scene saved successfully.");

   return true;
}

bool WorldSerializer::check_if_scene_exists(const std::string& scene_name)
{
   const std::ifstream reader(SCENES_FOLDER_PATH + scene_name + ".txt");
   return reader.is_open();
}