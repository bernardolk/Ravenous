#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <engine/core/rvn_types.h>
#include "engine/rvn.h"
#include <rvn_macros.h>
#include <engine/logging.h>
#include <engine/serialization/sr_entity.h>
#include <engine/collision/collision_mesh.h>
#include <glm/gtx/quaternion.hpp>
#include "engine/collision/primitives/bounding_box.h"
#include "engine/mesh.h"
#include "engine/entity.h"
#include "engine/entity_pool.h"
#include <engine/entity_manager.h>
#include <engine/serialization/sr_common.h>
#include "rvn_macros.h"
#include "player.h"
#include "engine/world/world.h"
#include "engine/camera.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_config.h"

#include <fstream>


ProgramConfig ConfigSerializer::load_configs()
{
   auto p = Parser{CONFIG_FILE_PATH};
   auto config = ProgramConfig();

   while(p.next_line())
   {
      p.parse_token();
      const auto attribute = get_parsed<std::string>(p);

      p.parse_all_whitespace();
      p.parse_symbol();
      
      if(get_parsed<char>(p) != '=')
      {
         std::cout << 
            "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << 
            CONFIG_FILE_PATH  << 
            "') LINE NUMBER " << 
            p.line_count << "\n";
            
         assert(false);
      }
      
      if(attribute == "scene")
      {
         p.parse_all_whitespace();
         p.parse_token();
         config.initial_scene = get_parsed<std::string>(p);
      }
      else if(attribute == "camspeed")
      {
         p.parse_all_whitespace();
         p.parse_float();
         config.camspeed = get_parsed<float>(p);
      }
      else if(attribute == "ambient_light")
      {
         p.parse_all_whitespace();
         p.parse_vec3();
         config.ambient_light = get_parsed<glm::vec3>(p);
      }
      else if(attribute == "ambient_intensity")
      {
         p.parse_all_whitespace();
         p.parse_float();
         config.ambient_intensity = get_parsed<float>(p);;
      }
   }

   return config;
}

void ConfigSerializer::parse_camera_settings(Parser& p)
{
      p.parse_all_whitespace();
      p.parse_vec3();
      scene_info->camera->Position = get_parsed<glm::vec3>(p);

      p.parse_all_whitespace();
      p.parse_vec3();
      camera_look_at(scene_info->camera, get_parsed<glm::vec3>(p), false);
}


bool ConfigSerializer::save(const ProgramConfig& config)
{
   std::ofstream writer(CONFIG_FILE_PATH);
   if(!writer.is_open())
   {
      std::cout << "Saving config file failed.\n";
      return false;
   }

   writer << "scene = " << config.initial_scene << "\n";
   writer << "camspeed = " << config.camspeed << "\n";
   writer << "ambient_light = " 
      << config.ambient_light.x << " "
      << config.ambient_light.y << " "
      << config.ambient_light.z << "\n";
      
   writer << "ambient_intensity = " << config.ambient_intensity << "\n";

   writer.close();
   std::cout << "Config file saved succesfully.\n";

   return true;
}
