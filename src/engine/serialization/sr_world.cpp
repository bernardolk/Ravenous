#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <engine/core/rvn_types.h>
#include <rvn_macros.h>
#include <engine/logging.h>
#include <engine/rvn.h>
#include <player.h>
#include <engine/entity.h>
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
   world.clear(&manager);

   // Gets a new world struct from scratch
   world.init();

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

   G_SCENE_INFO.player->entity_ptr = manager.create_entity({
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
   else                 manager.next_entity_id     = get_parsed<u64>(p);

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
            assert(false);
      }
   }

   // -----------------------------------
   //          Post parse steps
   // -----------------------------------
   world.player->update(&world, true);
   CL_update_player_world_cells(G_SCENE_INFO.player, &world);

   // connects entities using deferred load buffer
   For(entity_relations.count)
   {
      //@TODO: wtf is going on with 2 entity vars being declared here?
      Entity* entity                = entity_relations.entities[i];
      auto relation                 = entity_relations.relations[i];
      auto deferred_entity_id    = entity_relations.deferred_entity_ids[i];
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
      manager.next_entity_id = Max_Entity_Id + 1;
   }

   // assign IDs to entities missing them starting from max current id
   For(world.entities.size())
   {
      if(auto entity = world.entities[i]; entity->name != PLAYER_NAME && entity->id == -1)
      {
         entity->id = manager.next_entity_id++;
      }
   }

   // clear static relations buffer
   EntitySerializer::_clear_buffer();
   
   world.update_cells_in_use_list();

   G_SCENE_INFO.scene_name = filename;

   // save backup
   // save_scene_to_file("backup", G_SCENE_INFO.player, world, true);

   return true;
}


bool WorldSerializer::save_to_file(std::string& filename = "", const bool do_copy = false)
{
   const bool was_renamed = filename.length() > 0;
   if(!was_renamed)
      filename = G_SCENE_INFO.scene_name;

   if(do_copy && !was_renamed)
   {
      std::cout << "please provide a name for the copy.\n";
      return false;
   }

   const auto path = SCENES_FOLDER_PATH + filename + ".txt";
   std::ofstream writer(path);
   
   if(!writer.is_open())
   {
      std::cout << "Saving scene failed.\n";
      return false;
   }

   writer << std::fixed << std::setprecision(4);

   writer << "NEXT_ENTITY_ID = " << manager.next_entity_id << "\n";

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

   
   // write lights to file
   for(auto& light : world->point_lights)
      LightSerializer::save(light);

   for(auto& light : world->spot_lights)
      LightSerializer::save(light);

   for(auto& light : world->directional_lights)
      LightSerializer::save(light);
   

   for(int i = 0; i < world->entities.size(); i++) 
   {
      Entity* entity = world->entities[i];
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

         case EntityType_TimerTrigger:
         {
            writer << "type timer_trigger\n";
            writer << "trigger " 
               << entity->trigger_scale.x << " "
               << entity->trigger_scale.y << " "
               << entity->trigger_scale.z << "\n";
            if(entity->timer_trigger_data.timer_target != nullptr)
               writer << "timer_target " << entity->timer_trigger_data.timer_target->id << "\n";
            writer << "timer_duration " << entity->timer_trigger_data.timer_duration << "\n";

            For(entity->timer_trigger_data.size)
            {
               auto marking            = entity->timer_trigger_data.markings[i];
               u32  time_checkpoint    = entity->timer_trigger_data.time_checkpoints[i];
               if(marking != nullptr)
                  writer << "timer_marking " << marking->id << " " << time_checkpoint << "\n";
            }

            break;
         }

         case EntityType_TimerTarget:
         {
            writer << "type timer_target\n";
            writer << "timer_target_type " << entity->timer_target_data.timer_target_type << "\n";

            if(entity->timer_target_data.timer_start_animation != 0)
               writer << "timer_start_animation " << entity->timer_target_data.timer_start_animation << "\n";

            if(entity->timer_target_data.timer_stop_animation != 0)
               writer << "timer_stop_animation " << entity->timer_target_data.timer_stop_animation << "\n";

            break;
         }

         case EntityType_TimerMarking:
         {
            writer << "type timer_marking\n";
            writer << "timer_marking_color_on "
               << entity->timer_marking_data.color_on.x << " "
               << entity->timer_marking_data.color_on.y << " "
               << entity->timer_marking_data.color_on.z << "\n";

            writer << "timer_marking_color_off "
               << entity->timer_marking_data.color_off.x << " "
               << entity->timer_marking_data.color_off.y << " "
               << entity->timer_marking_data.color_off.z << "\n";

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
   
    return true;
}