#include <string>
#include <iostream>
#include <fstream>
#include <engine/core/types.h>
#include "engine/entities/entity.h"
#include "game/entities/player.h"
#include "engine/world/world.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_player.h"


void PlayerSerializer::ParseAttribute(Parser& p)
{
	Player* player = T_World::Get()->player;

	p.ParseToken();
	const auto attribute = GetParsed<std::string>(p);

	p.ParseAllWhitespace();
	p.ParseSymbol();

	if (GetParsed<char>(p) != '=')
	{
		std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << p.line_count << "\n";
		assert(false);
	}

	if (attribute == "player_position")
	{
		p.ParseVec3();
		const auto position = GetParsed<glm::vec3>(p);
		player->position = position;
		player->checkpoint_pos = position;
		player->height_before_fall = position.y;
	}

	else if (attribute == "player_initial_velocity")
	{
		p.ParseVec3();
		player->initial_velocity = GetParsed<glm::vec3>(p);
		player->velocity = player->initial_velocity;
	}

	else if (attribute == "player_state")
	{
		p.ParseAllWhitespace();
		p.ParseInt();
		player->initial_player_state = static_cast<PlayerState>(GetParsed<u32>(p));
		player->player_state = player->initial_player_state;
	}

	else if (attribute == "player_fall_speed")
	{
		p.ParseAllWhitespace();
		p.ParseFloat();
		player->fall_speed = GetParsed<float>(p);
	}

	else if (attribute == "player_fall_acceleration")
	{
		p.ParseAllWhitespace();
		p.ParseFloat();
		player->fall_acceleration = GetParsed<float>(p);
	}
	else
	{
		std::cout << "UNRECOGNIZED ATTRIBUTE AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << p.line_count << "\n";
	}
}

void PlayerSerializer::ParseOrientation(Parser& p)
{
	Player* player = T_World::Get()->player;

	p.ParseToken();
	if (GetParsed<std::string>(p) == "player_orientation")
	{
		p.ParseAllWhitespace();
		p.ParseSymbol();

		if (GetParsed<char>(p) != '=')
		{
			std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << p.line_count << "\n";
			assert(false);
		}

		p.ParseVec3();
		player->orientation = GetParsed<glm::vec3>(p);
	}
	else
		assert(false);
}

void PlayerSerializer::Save(std::ofstream& writer)
{
	Player* player = T_World::Get()->player;
	
	writer << "@player_position = "
	<< player->position.x << " "
	<< player->position.y << " "
	<< player->position.z << "\n";
	writer << "@player_initial_velocity = "
	<< player->initial_velocity.x << " "
	<< player->initial_velocity.y << " "
	<< player->initial_velocity.z << "\n";

	if (player->player_state == PlayerState::Standing)
		writer << "@player_state = " << static_cast<int>(PlayerState::Standing) << "\n";
	else
		writer << "@player_state = " << static_cast<int>(player->initial_player_state) << "\n";

	writer << "@player_fall_acceleration = " << player->fall_acceleration << "\n";
}
