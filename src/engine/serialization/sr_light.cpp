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
#include <engine/lights.h>
#include "engine/world/world.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_light.h"

#include <fstream>


void LightSerializer::parse(Parser& p)
{
   p.parse_token();
   const auto type = get_parsed<std::string>(p);

   if (type == "point")
      _parse_point_light(p);

   else if (type == "spot")
      _parse_spot_light(p);

   else if (type == "directional")
      _parse_directional_light(p);

   else
   {
      std::cout << "FATAL: Unrecognized light source in scene file '" << p.filepath << "', line " << p.line_count << ".\n";
      assert(false);
   }
}

void LightSerializer::_parse_point_light(Parser& p)
{
   //@TODO: Deal with this with a memory pool (?)
   PointLight& point_light = *(new PointLight());

   while(p.next_line())
   {
      p.parse_token();
      const std::string property = get_parsed<std::string>(p);

      if(property == "position")
      {
         p.parse_vec3();
         point_light.position = get_parsed<glm::vec3>(p);
      }
      
      else if(property == "diffuse")
      {
         p.parse_vec3();
         point_light.diffuse = get_parsed<glm::vec3>(p);
      }
      
      else if(property == "specular")
      {
         p.parse_vec3();
         point_light.specular = get_parsed<glm::vec3>(p);
      }
      
      else if(property == "constant")
      {
         p.parse_all_whitespace();
         p.parse_float();
         point_light.intensity_constant = get_parsed<float>(p);
      }
      
      else if(property == "linear")
      {
         p.parse_all_whitespace();
         p.parse_float();
         point_light.intensity_linear = get_parsed<float>(p);
      }
      
      else if(property == "quadratic")
      {
         p.parse_all_whitespace();
         p.parse_float();
         point_light.intensity_quadratic = get_parsed<float>(p);
      }
      
      else break;
   }

   world->point_lights.push_back(&point_light);
}

void LightSerializer::_parse_spot_light(Parser& p)
{
   //@TODO: Deal with this with a memory pool (?)
   SpotLight& spotlight = *(new SpotLight());

   while(p.next_line())
   {
      p.parse_token();
      const auto property = get_parsed<std::string>(p);

      if(property == "position")
      {
         p.parse_vec3();
         spotlight.position = get_parsed<glm::vec3>(p);;
      }
      else if(property == "direction")
      {
         p.parse_vec3();
         spotlight.direction = get_parsed<glm::vec3>(p);;
      }
      else if(property == "diffuse")
      {
         p.parse_vec3();
         spotlight.diffuse = get_parsed<glm::vec3>(p);;
      }
      else if(property == "specular")
      {
         p.parse_vec3();
         spotlight.specular = get_parsed<glm::vec3>(p);;
      }
      else if(property == "constant")
      {
         p.parse_all_whitespace();
         p.parse_float();
         spotlight.intensity_constant = get_parsed<float>(p);
      }
      else if(property == "linear")
      {
         p.parse_all_whitespace();
         p.parse_float();
         spotlight.intensity_linear = get_parsed<float>(p);
      }
      else if(property == "quadratic")
      {
         p.parse_all_whitespace();
         p.parse_float();
         spotlight.intensity_quadratic = get_parsed<float>(p);
      }
      else if(property == "innercone")
      {
         p.parse_all_whitespace();
         p.parse_float();
         spotlight.innercone = get_parsed<float>(p);
      }
      else if(property == "outercone")
      {
         p.parse_all_whitespace();
         p.parse_float();
         spotlight.outercone = get_parsed<float>(p);
      }
      else break;
   }

   world->spot_lights.push_back(&spotlight);
}

void LightSerializer::_parse_directional_light(Parser& p)
{
   //@TODO: Deal with this with a memory pool (?)
   DirectionalLight& light = *(new DirectionalLight());

   while(p.next_line())
   {
      p.parse_token();
      const auto property = get_parsed<std::string>(p);;

      if(property == "direction")
      {
         p.parse_vec3();
         light.direction = get_parsed<glm::vec3>(p);;
      }
      else if(property == "diffuse")
      {
         p.parse_vec3();
         light.diffuse = get_parsed<glm::vec3>(p);;
      }
      else if(property == "specular")
      {
         p.parse_vec3();
         light.specular = get_parsed<glm::vec3>(p);;
      }
      else break;
   }

   world->directional_lights.push_back(&light);
}

void save(std::ofstream& writer, const PointLight* light)
{
   writer << "\n$point\n"
           << "position "
           << light->position.x << " "
           << light->position.y << " "
           << light->position.z << "\n"
           << "diffuse "
           << light->diffuse.x << " "
           << light->diffuse.y << " "
           << light->diffuse.z << "\n"
           << "specular "
           << light->specular.x << " "
           << light->specular.y << " "
           << light->specular.z << "\n"
           << "constant "
           << light->intensity_constant << "\n"
           << "linear "
           << light->intensity_linear << "\n"
           << "quadratic "
           << light->intensity_quadratic << "\n";
}

void save(std::ofstream& writer, const SpotLight* light)
{
   writer << "\n$spot\n"
         << "position "
         << light->position.x << " "
         << light->position.y << " "
         << light->position.z << "\n"
         << "direction "
         << light->direction.x << " "
         << light->direction.y << " "
         << light->direction.z << "\n"
         << "diffuse "
         << light->diffuse.x << " "
         << light->diffuse.y << " "
         << light->diffuse.z << "\n"
         << "specular "
         << light->specular.x << " "
         << light->specular.y << " "
         << light->specular.z << "\n"
         << "innercone "
         << light->innercone << "\n"
         << "outercone "
         << light->outercone << "\n"
         << "constant "
         << light->intensity_constant << "\n"
         << "linear "
         << light->intensity_linear << "\n"
         << "quadratic "
         << light->intensity_quadratic << "\n";
}


void save(std::ofstream& writer, const DirectionalLight* light)
{
   writer << "\n$directional\n"
         << "direction "
         << light->direction.x << " "
         << light->direction.y << " "
         << light->direction.z << "\n"
         << "diffuse "
         << light->diffuse.x << " "
         << light->diffuse.y << " "
         << light->diffuse.z << "\n"
         << "specular "
         << light->specular.x << " "
         << light->specular.y << " "
         << light->specular.z << "\n";
}