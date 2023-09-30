#include <string>
#include <iostream>
#include <fstream>
#include <engine/core/types.h>
#include "game/entities/EPlayer.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_player.h"
#include "engine/world/World.h"


void PlayerSerializer::ParseAttribute(Parser& Parse)
{
	EPlayer* Player = EPlayer::Get();

	Parse.ParseToken();
	const auto Attribute = GetParsed<std::string>(Parse);

	Parse.ParseAllWhitespace();
	Parse.ParseSymbol();

	if (GetParsed<char>(Parse) != '=')
	{
		std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << Parse.Filepath << "') LINE NUMBER " << Parse.LineCount << "\n";
		assert(false);
	}

	if (Attribute == "player_position")
	{
		Parse.ParseVec3();
		const auto Position = GetParsed<glm::vec3>(Parse);
		Player->Position = Position;
		Player->CheckpointPos = Position;
		Player->HeightBeforeFall = Position.y;
	}

	else if (Attribute == "player_initial_velocity")
	{
		Parse.ParseVec3();
		Player->InitialVelocity = GetParsed<glm::vec3>(Parse);
		Player->Velocity = Player->InitialVelocity;
	}

	else if (Attribute == "player_state")
	{
		Parse.ParseAllWhitespace();
		Parse.ParseInt();
		Player->InitialPlayerState = static_cast<NPlayerState>(GetParsed<uint>(Parse));
		Player->PlayerState = Player->InitialPlayerState;
	}

	else
	{
		std::cout << "UNRECOGNIZED ATTRIBUTE AT SCENE DESCRIPTION FILE ('" << Parse.Filepath << "') LINE NUMBER " << Parse.LineCount << "\n";
	}
}

void PlayerSerializer::ParseOrientation(Parser& Parse)
{
	EPlayer* Player = EPlayer::Get();

	Parse.ParseToken();
	if (GetParsed<std::string>(Parse) == "player_orientation")
	{
		Parse.ParseAllWhitespace();
		Parse.ParseSymbol();

		if (GetParsed<char>(Parse) != '=')
		{
			std::cout << "SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" << Parse.Filepath << "') LINE NUMBER " << Parse.LineCount << "\n";
			assert(false);
		}

		Parse.ParseVec3();
		Player->Orientation = GetParsed<glm::vec3>(Parse);
	}
	else
		assert(false);
}

void PlayerSerializer::Save(std::ofstream& Writer)
{
	EPlayer* Player = EPlayer::Get();

	Writer << "@player_position = "
	<< Player->Position.x << " "
	<< Player->Position.y << " "
	<< Player->Position.z << "\n";
	Writer << "@player_initial_velocity = "
	<< Player->InitialVelocity.x << " "
	<< Player->InitialVelocity.y << " "
	<< Player->InitialVelocity.z << "\n";

	if (Player->PlayerState == NPlayerState::Standing)
		Writer << "@player_state = " << static_cast<int>(NPlayerState::Standing) << "\n";
	else
		Writer << "@player_state = " << static_cast<int>(Player->InitialPlayerState) << "\n";
}
