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
	const auto Path = Paths::Scenes + filename + ".txt";

	world = RWorld::Get();

	// Set player's assets
	{
		REntityAttributes Attrs;
		Attrs.Name = "Player";
		Attrs.Mesh = "capsule";
		Attrs.Shader = "model";
		Attrs.Texture = "pink";
		Attrs.CollisionMesh = "capsule";
		Attrs.Scale = vec3(1);

		SetEntityAssets(EPlayer::Get(), Attrs);
	}

	// creates deferred load buffer for associating entities after loading them all
	auto EntityRelations = DeferredEntityRelationBuffer();

	// starts reading
	auto P = Parser{path};

	// -----------------------------------
	//           Parse entities
	// -----------------------------------
	while (p.NextLine())
	{
		p.ParseSymbol();
		switch (GetParsed<char>(p))
		{
			case '#':
				EntitySerializer::Parse(p);
				break;

			case '@':
				PlayerSerializer::ParseAttribute(p);
				break;

			case '$':
				LightSerializer::Parse(p);
				break;

			case '*':
				ConfigSerializer::ParseCameraSettings(p);
				break;

			case '&':
				PlayerSerializer::ParseOrientation(p);
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
	CL_UpdatePlayerWorldCells(Player);

	// connects entities using deferred load buffer
	For(EntityRelations.count)
	{
		EEntity* DeferredEntity = nullptr;
		auto DeferredEntityId = EntityRelations.deferred_entity_ids[i];

		auto EntityIterator = world->GetEntityIterator();
		while (auto* Entity = EntityIterator())
		{
			if (Entity->id == DeferredEntityId)
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

	RWorld::Get()->SceneName = filename;

	return true;
}

bool WorldSerializer::SaveToFile()
{
	const std::string F;
	return SaveToFile(f, false);
}

bool WorldSerializer::SaveToFile(const std::string& NewFilename, const bool DoCopy = false)
{
	std::string Filename = new_filename;
	if (new_filename.empty())
	{
		filename = RWorld::Get()->SceneName;

		if (DoCopy)
		{
			print("please provide a name for the copy.");
			return false;
		}
	}

	const auto Path = Paths::Scenes + filename + ".txt";
	std::ofstream writer(path);

	if (!writer.is_open())
	{
		print("Saving scene failed.");
		return false;
	}

	writer << std::fixed << std::setprecision(4);

	DEPRECATED_LINE
	//writer << "NEXT_ENTITY_ID = " << manager->next_entity_id << "\n";


	// @TODO: Refactor this at some point
	// write camera settings to file
	const auto* Camera = RCameraManager::Get()->GetEditorCamera();
	writer << "*"
	<< camera->position.x << " "
	<< camera->position.y << " "
	<< camera->position.z << "  "
	<< camera->front.x << " "
	<< camera->front.y << " "
	<< camera->front.z << "\n";

	// write player attributes to file
	PlayerSerializer::Save(writer);

	// @TODO: Refactor this at some point
	// write player orientation
	const auto* GameCam = RCameraManager::Get()->GetGameCamera();
	writer << "&player_orientation = "
	<< game_cam->front.x << " "
	<< game_cam->front.y << " "
	<< game_cam->front.z << "\n";

	// write lights to file
	for (const auto& light : world->point_lights)
		LightSerializer::Save(writer, light);

	for (const auto& light : world->spot_lights)
		LightSerializer::Save(writer, light);

	for (const auto& light : world->directional_lights)
		LightSerializer::Save(writer, light);

	auto EntityIter = world->GetEntityIterator();
	while (auto* Entity = EntityIter())
	{
		EntitySerializer::Save(writer, *entity);
	}

	writer.close();

	if (DoCopy)
	{
		Log(LOG_INFO, "Scene copy saved successfully as '" + filename + ".txt'");
	}
	else if (!new_filename.empty())
	{
		Log(LOG_INFO, "Scene saved successfully as '" + filename + ".txt' (now editing it)");
		RWorld::Get()->SceneName = filename;
	}
	else
		Log(LOG_INFO, "Scene saved successfully.");

	return true;
}

bool WorldSerializer::CheckIfSceneExists(const std::string& SceneName)
{
	const std::ifstream Reader(Paths::Scenes + scene_name + ".txt");
	return reader.is_open();
}
