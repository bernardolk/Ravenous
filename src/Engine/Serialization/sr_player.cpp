#include <string>
#include <iostream>
#include <fstream>
#include <engine/core/types.h>
#include "game/entities/EPlayer.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_player.h"
#include "engine/world/World.h"


void PlayerSerializer::ParseAttribute(Parser& p)
{
	EPlayer* player = EPlayer::Get();

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
		player->initial_player_state = static_cast<NPlayerState>(GetParsed<uint>(p));
		player->player_state = player->initial_player_state;
	}

	else
	{
		std::cout << "UNRECOGNIZED ATTRIBUTE AT SCENE DESCRIPTION FILE ('" << p.filepath << "') LINE NUMBER " << p.line_count << "\n";
	}
}

void PlayerSerializer::ParseOrientation(Parser& p)
{
	EPlayer* player = EPlayer::Get();

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
	EPlayer* player = EPlayer::Get();
	
	writer << "@player_position = "
	<< player->position.x << " "
	<< player->position.y << " "
	<< player->position.z << "\n";
	writer << "@player_initial_velocity = "
	<< player->initial_velocity.x << " "
	<< player->initial_velocity.y << " "
	<< player->initial_velocity.z << "\n";

	if (player->player_state == NPlayerState::Standing)
		writer << "@player_state = " << static_cast<int>(NPlayerState::Standing) << "\n";
	else
		writer << "@player_state = " << static_cast<int>(player->initial_player_state) << "\n";
}
