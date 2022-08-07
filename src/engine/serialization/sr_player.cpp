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
#include "player.h"
#include "engine/world/world.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_player.h"



void PlayerSerializer::parse_attribute(Parser& p)
{
   Player* player = world.player;
   
   p.parse_token();
   const auto attribute = get_parsed<std::string>(p);

   p.parse_all_whitespace();
   p.parse_symbol();

   if(get_parsed<char>(p) != '=')
   {
      std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << line_count << "\n";
      assert(false);
   }

   if(attribute == "player_position")
   {
      p.parse_vec3();
      const auto position = get_parsed<glm::vec3>(p);
      player->entity_ptr->position     = position;
      player->checkpoint_pos           = position;
      player->height_before_fall       = position.y;
   }
   
   else if(attribute == "player_initial_velocity")
   {
      p.parse_vec3();
      player->initial_velocity = get_parsed<glm::vec3>(p);
      player->entity_ptr->velocity = player->initial_velocity;
   }
   
   else if(attribute == "player_state")
   {
      p.parse_all_whitespace();
      p.parse_int();
      player->initial_player_state = (PlayerState) get_parsed<u32>(p);
      player->player_state = player->initial_player_state;
   }
   
   else if(attribute == "player_fall_speed")
   {
      p.parse_all_whitespace();
      p.parse_float();
      player->fall_speed = get_parsed<float>(p);
   }
   
   else if(attribute == "player_fall_acceleration")
   {
      p.parse_all_whitespace();
      p.parse_float();
      player->fall_acceleration = get_parsed<float>(p);
   }
   else
   {
      std::cout << "UNRECOGNIZED ATTRIBUTE AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << line_count << "\n";
   }
}


void PlayerSerializer::parse_orientation(Parser& p)
{
   Player* player = world.player;
   
   p.parse_token();
   if(get_parsed<std::string>(p) == "player_orientation")
   {
      p.parse_all_whitespace();
      p.parse_symbol();

      if(get_parsed<char>(p) != '=')
      {
         std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << line_count << "\n";
         assert(false);
      }

      p.parse_vec3();
      player->orientation = get_parsed<glm::vec3>(p);
   }
   else assert(false);
}


void PlayerSerializer::save()
{
      
}