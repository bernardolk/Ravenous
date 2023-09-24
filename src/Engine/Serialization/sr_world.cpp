#include "sr_world.h"

#include <iomanip>

#include "game/entities/player.h"
#include "sr_config.h"
#include "sr_entity.h"
#include "sr_light.h"
#include "sr_player.h"
#include "engine/catalogues.h"
#include "engine/camera/camera.h"
#include "engine/core/logging.h"
#include "engine/rvn.h"
#include "engine/collision/ClController.h"
#include "engine/world/world.h"
#include "parsing/parser.h"

bool WorldSerializer::LoadFromFile(const string& filename)
{
	const auto path = Paths::Scenes + filename + ".txt";

	world = World::Get();

	// Set player's assets
	{
		EntityAttributes attrs;
		attrs.name = "Player";
		attrs.mesh = "capsule";
		attrs.shader = "model";
		attrs.texture = "pink";
		attrs.collision_mesh = "capsule";
		attrs.scale = vec3(1);
		
		SetEntityAssets(Player::Get(), attrs);
	}

	// creates deferred load buffer for associating entities after loading them all
	auto entity_relations = DeferredEntityRelationBuffer();

	// starts reading
	auto p = Parser{path};

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
	Player* player = Player::Get();
	player->Update();

	// TODO: Address
	CL_UpdatePlayerWorldCells(player);

	// connects entities using deferred load buffer
	For(entity_relations.count)
	{
		EEntity* deferred_entity = nullptr;
		auto deferred_entity_id = entity_relations.deferred_entity_ids[i];

		auto entity_iterator = world->GetEntityIterator();
		while (auto* entity = entity_iterator())
		{
			if (entity->id == deferred_entity_id)
			{
				deferred_entity = entity;
				break;
			}
		}

		if (deferred_entity == nullptr)
			fatal_error("Entity with id '%llu' not found to stablish a defined entity relationship.", deferred_entity_id);
	}
	
	// clear static relations buffer
	EntitySerializer::ClearBuffer();

	World::Get()->scene_name = filename;
	
	return true;
}

bool WorldSerializer::SaveToFile()
{
	const std::string f;
	return SaveToFile(f, false);
}

bool WorldSerializer::SaveToFile(const std::string& new_filename, const bool do_copy = false)
{
	std::string filename = new_filename;
	if (new_filename.empty())
	{
		filename = World::Get()->scene_name;

		if (do_copy)
		{
			print("please provide a name for the copy.");
			return false;
		}
	}

	const auto path = Paths::Scenes + filename + ".txt";
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
	const auto* camera = CameraManager::Get()->GetEditorCamera();
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
	const auto* game_cam = CameraManager::Get()->GetGameCamera();
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

	auto entity_iter = world->GetEntityIterator();
	while (auto* entity = entity_iter())
	{
		EntitySerializer::Save(writer, *entity);
	}

	writer.close();

	if (do_copy)
	{
		Log(LOG_INFO, "Scene copy saved successfully as '" + filename + ".txt'");
	}
	else if (!new_filename.empty())
	{
		Log(LOG_INFO, "Scene saved successfully as '" + filename + ".txt' (now editing it)");
		World::Get()->scene_name = filename;
	}
	else
		Log(LOG_INFO, "Scene saved successfully.");

	return true;
}

bool WorldSerializer::CheckIfSceneExists(const std::string& scene_name)
{
	const std::ifstream reader(Paths::Scenes + scene_name + ".txt");
	return reader.is_open();
}
