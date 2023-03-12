#include "sr_world.h"

#include "game/entities/player.h"
#include "sr_config.h"
#include "sr_entity.h"
#include "sr_light.h"
#include "sr_player.h"
#include "engine/camera/camera.h"
#include "engine/entities/manager/entity_manager.h"
#include "engine/core/logging.h"
#include "engine/rvn.h"
#include "engine/collision/cl_controller.h"
#include "engine/world/scene_manager.h"
#include "engine/world/world.h"
#include "parsing/parser.h"


bool WorldSerializer::LoadFromFile(const std::string& filename)
{
	const auto path = Paths::Scenes + filename + ".txt";

	// clears the current scene entity data
	world->Clear(manager);

	// Gets a new world struct from scratch
	world->Init();

	// Either creates new scene or resets current structures
	GlobalSceneInfo::RefreshActiveScene();
	world->player = Player::Get();

	// Creates player
	Player::Get()->entity_ptr = manager->CreateEntity({
		.name = PlayerName,
		.mesh = "capsule",
		.shader = "model",
		.texture = "pink",
		.collision_mesh = "capsule",
		.scale = vec3(1)
		});

	// creates deferred load buffer for associating entities after loading them all
	auto entity_relations = DeferredEntityRelationBuffer();

	// starts reading
	auto p = Parser{path};

	// parses header
	p.NextLine();
	p.ParseToken();
	if (!p.HasToken())
		Quit_fatal("Scene '" + filename + "' didn't start with NEXT_ENTITY_ID token.")

	const auto next_entity_id_token = get_parsed<std::string>(p);
	if (next_entity_id_token != "NEXT_ENTITY_ID")
		Quit_fatal("Scene '" + filename + "' didn't start with NEXT_ENTITY_ID token.")

	p.ParseWhitespace();
	p.ParseSymbol();
	if (!p.HasToken() || get_parsed<char>(p) != '=')
		Quit_fatal("Missing '=' after NEXT_ENTITY_ID.")

	p.ParseWhitespace();
	p.ParseU64();

	// ENTITY IDs related code
	bool recompute_next_entity_id = false;
	if (!p.HasToken())
		recompute_next_entity_id = true;
	else
		manager->next_entity_id = get_parsed<u64>(p);

	// -----------------------------------
	//           Parse entities
	// -----------------------------------
	while (p.NextLine())
	{
		p.ParseSymbol();
		switch (get_parsed<char>(p))
		{
			case '#':
				EntitySerializer::parse(p);
				break;

			case '@':
				PlayerSerializer::parse_attribute(p);
				break;

			case '$':
				LightSerializer::parse(p);
				break;

			case '*':
				ConfigSerializer::parse_camera_settings(p);
				break;

			case '&':
				PlayerSerializer::parse_orientation(p);
				break;

			default:
				break;
		}
	}

	// -----------------------------------
	//          Post parse steps
	// -----------------------------------
	world->player->Update(world, true);
	CL_UpdatePlayerWorldCells(Player::Get(), world);

	// connects entities using deferred load buffer
	For(entity_relations.count)
	{
		//@TODO: wtf is going on with 2 entity vars being declared here?
		Entity* entity = entity_relations.entities[i];
		Entity* deferred_entity = nullptr;
		auto relation = entity_relations.relations[i];
		auto deferred_entity_id = entity_relations.deferred_entity_ids[i];

		for (const auto entity_b : world->entities)
		{
			if (entity_b->id == deferred_entity_id)
			{
				deferred_entity = entity_b;
				break;
			}
		}

		if (deferred_entity == nullptr)
			Quit_fatal("Entity with id '" + std::to_string(deferred_entity_id) + "' not found to stablish a defined entity relationship.")

		switch (relation)
		{
			case SrEntityRelation_TimerTarget:
			{
				entity->timer_trigger_data.timer_target = deferred_entity;
				break;
			}

			case SrEntityRelation_TimerMarking:
			{
				u32 time_checkpoint = entity_relations.aux_uint_buffer[i];
				entity->timer_trigger_data.AddMarking(deferred_entity, time_checkpoint);
				break;
			}
		}
	}


	// -----------------------------------
	//         Entity id bookkeeping
	// -----------------------------------

	// If missing NEXT_ENTITY_ID in scene header, recompute from collected Ids (If no entity has an ID yet, this will be 1)
	if (recompute_next_entity_id)
	{
		manager->next_entity_id = Max_Entity_Id + 1;
	}

	// assign IDs to entities missing them starting from max current id
	For(world->entities.size())
	{
		if (auto entity = world->entities[i]; entity->name != PlayerName && entity->id == -1)
		{
			entity->id = manager->next_entity_id++;
		}
	}

	// clear static relations buffer
	EntitySerializer::_clear_buffer();

	GlobalSceneInfo::Get()->scene_name = filename;

	world->UpdateEntities();
	world->UpdateCellsInUseList();

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
		filename = GlobalSceneInfo::Get()->scene_name;

		if (do_copy)
		{
			std::cout << "please provide a name for the copy.\n";
			return false;
		}
	}

	const auto path = Paths::Scenes + filename + ".txt";
	std::ofstream writer(path);

	if (!writer.is_open())
	{
		std::cout << "Saving scene failed.\n";
		return false;
	}

	writer << std::fixed << std::setprecision(4);

	writer << "NEXT_ENTITY_ID = " << manager->next_entity_id << "\n";

	// @TODO: Refactor this at some point
	// write camera settings to file
	const auto camera = ConfigSerializer::scene_info->views[0];
	writer << "*"
	<< camera->position.x << " "
	<< camera->position.y << " "
	<< camera->position.z << "  "
	<< camera->front.x << " "
	<< camera->front.y << " "
	<< camera->front.z << "\n";

	// write player attributes to file
	PlayerSerializer::save(writer);

	// @TODO: Refactor this at some point
	// write player orientation
	const auto fps_cam = GlobalSceneInfo::Get()->views[GameCam];
	writer << "&player_orientation = "
	<< fps_cam->front.x << " "
	<< fps_cam->front.y << " "
	<< fps_cam->front.z << "\n";

	// write lights to file
	for (const auto& light : world->point_lights)
		LightSerializer::save(writer, light);

	for (const auto& light : world->spot_lights)
		LightSerializer::save(writer, light);

	for (const auto& light : world->directional_lights)
		LightSerializer::save(writer, light);


	for (const auto& entity : world->entities)
	{
		if (entity->name == "Player")
			continue;

		EntitySerializer::save(writer, *entity);
	}

	writer.close();

	if (do_copy)
	{
		log(LOG_INFO, "Scene copy saved successfully as '" + filename + ".txt'");
	}
	else if (!new_filename.empty())
	{
		log(LOG_INFO, "Scene saved successfully as '" + filename + ".txt' (now editing it)");
		GlobalSceneInfo::Get()->scene_name = filename;
	}
	else
		log(LOG_INFO, "Scene saved successfully.");

	return true;
}

bool WorldSerializer::CheckIfSceneExists(const std::string& scene_name)
{
	const std::ifstream reader(Paths::Scenes + scene_name + ".txt");
	return reader.is_open();
}
