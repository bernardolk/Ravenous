#include "sr_world.h"

#include <iomanip>

#include "game/entities/EPlayer.h"
#include "sr_config.h"
#include "sr_entity.h"
#include "sr_light.h"
#include "sr_player.h"
#include "engine/catalogues.h"
#include "engine/camera/camera.h"
#include "engine/core/logging.h"
#include "engine/rvn.h"
#include "engine/collision/ClController.h"
#include "engine/world/World.h"
#include "parsing/parser.h"

bool WorldSerializer::LoadFromFile(const string& Filename)
{
	const auto Path = Paths::Scenes + Filename + ".txt";

	World = RWorld::Get();

	// Set player's assets
	{
		REntityAttributes Attrs;
		Attrs.Name = "Player";
		Attrs.Mesh = "capsule";
		Attrs.Shader = "model";
		Attrs.Texture = "pink";
		Attrs.CollisionMesh = "capsule";
		Attrs.Scale = vec3(1);

		auto* Player = EPlayer::Get();
		SetEntityAssets(Player, Attrs);
	}

	// creates deferred load buffer for associating entities after loading them all
	auto EntityRelations = DeferredEntityRelationBuffer();

	// starts reading
	auto Parse = Parser{Path};

	// -----------------------------------
	//           Parse entities
	// -----------------------------------
	while (Parse.NextLine())
	{
		Parse.ParseSymbol();
		switch (GetParsed<char>(Parse))
		{
			case '#':
				EntitySerializer::Parse(Parse);
				break;

			case '@':
				PlayerSerializer::ParseAttribute(Parse);
				break;

			case '$':
				LightSerializer::Parse(Parse);
				break;

			case '*':
				ConfigSerializer::ParseCameraSettings(Parse);
				break;

			case '&':
				PlayerSerializer::ParseOrientation(Parse);
				break;

			default:
				break;
		}
	}

	// -----------------------------------
	//          Post parse steps
	// -----------------------------------
	EPlayer* Player = EPlayer::Get();
	Player->Update();

	// TODO: Address
	ClUpdatePlayerWorldCells(Player);

	// connects entities using deferred load buffer
	for (int i = 0; i < EntityRelations.count; i++)
	{
		EEntity* DeferredEntity = nullptr;
		auto DeferredEntityId = EntityRelations.deferred_entity_ids[i];

		auto EntityIterator = World->GetEntityIterator();
		while (auto* Entity = EntityIterator())
		{
			if (Entity->ID == DeferredEntityId)
			{
				DeferredEntity = Entity;
				break;
			}
		}

		if (DeferredEntity == nullptr)
			fatal_error("Entity with id '%llu' not found to stablish a defined entity relationship.", DeferredEntityId);
	}

	// clear static relations buffer
	EntitySerializer::ClearBuffer();

	RWorld::Get()->SceneName = Filename;

	return true;
}

bool WorldSerializer::SaveToFile()
{
	const string F;
	return SaveToFile(F, false);
}

bool WorldSerializer::SaveToFile(const string& NewFilename, const bool DoCopy = false)
{
	string Filename = NewFilename;
	if (NewFilename.empty())
	{
		Filename = RWorld::Get()->SceneName;

		if (DoCopy)
		{
			print("please provide a name for the copy.");
			return false;
		}
	}

	const auto Path = Paths::Scenes + Filename + ".txt";
	std::ofstream Writer(Path);

	if (!Writer.is_open())
	{
		print("Saving scene failed.");
		return false;
	}

	Writer << std::fixed << std::setprecision(4);

	DEPRECATED_LINE
	//writer << "NEXT_ENTITY_ID = " << manager->next_entity_id << "\n";


	// @TODO: Refactor this at some point
	// write camera settings to file
	const auto* Camera = RCameraManager::Get()->GetEditorCamera();
	Writer << "*"
	<< Camera->Position.x << " "
	<< Camera->Position.y << " "
	<< Camera->Position.z << "  "
	<< Camera->Front.x << " "
	<< Camera->Front.y << " "
	<< Camera->Front.z << "\n";

	// write player attributes to file
	PlayerSerializer::Save(Writer);

	// @TODO: Refactor this at some point
	// write player orientation
	const auto* GameCam = RCameraManager::Get()->GetGameCamera();
	Writer << "&player_orientation = "
	<< GameCam->Front.x << " "
	<< GameCam->Front.y << " "
	<< GameCam->Front.z << "\n";

	// write lights to file
	for (const auto& Light : World->PointLights)
		LightSerializer::Save(Writer, Light);

	for (const auto& Light : World->SpotLights)
		LightSerializer::Save(Writer, Light);

	for (const auto& Light : World->DirectionalLights)
		LightSerializer::Save(Writer, Light);

	auto EntityIter = World->GetEntityIterator();
	while (auto* Entity = EntityIter())
	{
		EntitySerializer::Save(Writer, *Entity);
	}

	Writer.close();

	if (DoCopy)
	{
		Log(LOG_INFO, "Scene copy saved successfully as '" + Filename + ".txt'");
	}
	else if (!NewFilename.empty())
	{
		Log(LOG_INFO, "Scene saved successfully as '" + Filename + ".txt' (now editing it)");
		RWorld::Get()->SceneName = Filename;
	}
	else
		Log(LOG_INFO, "Scene saved successfully.");

	return true;
}

bool WorldSerializer::CheckIfSceneExists(const string& SceneName)
{
	//@std
	const std::ifstream Reader(Paths::Scenes + SceneName + ".txt");
	return Reader.is_open();
}
