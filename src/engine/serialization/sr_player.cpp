#include <string>
#include <iostream>
#include <fstream>
#include <engine/core/types.h>
#include "engine/entities/entity.h"
#include "game/entities/player.h"
#include "engine/world/world.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_player.h"


void PlayerSerializer::parse_attribute(Parser& p)
{
	Player* player = world->player;

	p.ParseToken();
	const auto attribute = get_parsed<std::string>(p);

	p.ParseAllWhitespace();
	p.ParseSymbol();

	if(get_parsed<char>(p) != '=')
	{
		std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << p.line_count << "\n";
		assert(false);
	}

	if(attribute == "player_position")
	{
		p.ParseVec3();
		const auto position = get_parsed<glm::vec3>(p);
		player->entity_ptr->position = position;
		player->checkpoint_pos = position;
		player->height_before_fall = position.y;
	}

	else if(attribute == "player_initial_velocity")
	{
		p.ParseVec3();
		player->initial_velocity = get_parsed<glm::vec3>(p);
		player->entity_ptr->velocity = player->initial_velocity;
	}

	else if(attribute == "player_state")
	{
		p.ParseAllWhitespace();
		p.ParseInt();
		player->initial_player_state = static_cast<PlayerState>(get_parsed<u32>(p));
		player->player_state = player->initial_player_state;
	}

	else if(attribute == "player_fall_speed")
	{
		p.ParseAllWhitespace();
		p.ParseFloat();
		player->fall_speed = get_parsed<float>(p);
	}

	else if(attribute == "player_fall_acceleration")
	{
		p.ParseAllWhitespace();
		p.ParseFloat();
		player->fall_acceleration = get_parsed<float>(p);
	}
	else
	{
		std::cout << "UNRECOGNIZED ATTRIBUTE AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << p.line_count << "\n";
	}
}

void PlayerSerializer::parse_orientation(Parser& p)
{
	Player* player = world->player;

	p.ParseToken();
	if(get_parsed<std::string>(p) == "player_orientation")
	{
		p.ParseAllWhitespace();
		p.ParseSymbol();

		if(get_parsed<char>(p) != '=')
		{
			std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << p.line_count << "\n";
			assert(false);
		}

		p.ParseVec3();
		player->orientation = get_parsed<glm::vec3>(p);
	}
	else
		assert(false);
}

void PlayerSerializer::save(std::ofstream& writer)
{
	const auto player = world->player;
	writer << "@player_position = "
	<< player->entity_ptr->position.x << " "
	<< player->entity_ptr->position.y << " "
	<< player->entity_ptr->position.z << "\n";
	writer << "@player_initial_velocity = "
	<< player->initial_velocity.x << " "
	<< player->initial_velocity.y << " "
	<< player->initial_velocity.z << "\n";

	if(player->player_state == PlayerState::Standing)
		writer << "@player_state = " << PlayerState::Standing << "\n";
	else
		writer << "@player_state = " << player->initial_player_state << "\n";

	writer << "@player_fall_acceleration = " << player->fall_acceleration << "\n";
}
