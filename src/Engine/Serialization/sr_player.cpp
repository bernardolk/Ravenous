#include <string>
#include <iostream>
#include <fstream>
#include <engine/core/types.h>
#include "game/entities/EPlayer.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_player.h"
#include "engine/world/World.h"


void PlayerSerializer::ParseAttribute(Parser& P)
{
	EPlayer* Player = EPlayer::Get();

	P.ParseToken();
	const auto Attribute = GetParsed<std::string>(P);

	P.ParseAllWhitespace();
	P.ParseSymbol();

	if (GetParsed<char>(P) != '=')
	{
		std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << P.Filepath << "') LINE NUMBER " << P.LineCount << "\n";
		assert(false);
	}

	if (Attribute == "player_position")
	{
		P.ParseVec3();
		const auto Position = GetParsed<glm::vec3>(p);
		Player->Position = position;
		Player->checkpoint_pos = position;
		Player->height_before_fall = position.y;
	}

	else if (Attribute == "player_initial_velocity")
	{
		P.ParseVec3();
		player->initial_velocity = GetParsed<glm::vec3>(p);
		Player->Velocity = Player->initial_velocity;
	}

	else if (Attribute == "player_state")
	{
		P.ParseAllWhitespace();
		P.ParseInt();
		Player->initial_player_state = static_cast<NPlayerState>(GetParsed<uint>(P));
		Player->player_state = Player->initial_player_state;
	}

	else
	{
		std::cout << "UNRECOGNIZED ATTRIBUTE AT SCENE DESCRIPTION FILE ('" << P.Filepath << "') LINE NUMBER " << P.LineCount << "\n";
	}
}

void PlayerSerializer::ParseOrientation(Parser& P)
{
	EPlayer* Player = EPlayer::Get();

	P.ParseToken();
	if (GetParsed<std::string>(P) == "player_orientation")
	{
		P.ParseAllWhitespace();
		P.ParseSymbol();

		if (GetParsed<char>(P) != '=')
		{
			std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << P.Filepath << "') LINE NUMBER " << P.LineCount << "\n";
			assert(false);
		}

		P.ParseVec3();
		player->orientation = GetParsed<glm::vec3>(p);
	}
	else
		assert(false);
}

void PlayerSerializer::Save(std::ofstream& Writer)
{
	EPlayer* Player = EPlayer::Get();

	writer << "@player_position = "
	<< Player->Position.x << " "
	<< Player->Position.y << " "
	<< Player->Position.z << "\n";
	writer << "@player_initial_velocity = "
	<< Player->initial_velocity.x << " "
	<< Player->initial_velocity.y << " "
	<< Player->initial_velocity.z << "\n";

	if (Player->player_state == NPlayerState::Standing)
		writer << "@player_state = " << static_cast<int>(NPlayerState::Standing) << "\n";
	else
		writer << "@player_state = " << static_cast<int>(Player->initial_player_state) << "\n";
}
