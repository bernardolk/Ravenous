#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <engine/core/types.h>
#include "engine/rvn.h"
#include <engine/logging.h>
#include <engine/serialization/sr_entity.h>
#include <engine/collision/collision_mesh.h>
#include <glm/gtx/quaternion.hpp>
#include "engine/collision/primitives/bounding_box.h"
#include "engine/mesh.h"
#include "engine/entity.h"
#include "engine/entity_pool.h"
#include <engine/entity_manager.h>
#include <engine/serialization/sr_common.h>
#include "player.h"
#include "engine/world/world.h"
#include "engine/camera.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_config.h"

#include <fstream>


ProgramConfig ConfigSerializer::load_configs()
{
	auto p = Parser{Paths::Config};
	auto config = ProgramConfig();

	while(p.NextLine())
	{
		p.ParseToken();
		const auto attribute = get_parsed<std::string>(p);

		p.ParseAllWhitespace();
		p.ParseSymbol();

		if(get_parsed<char>(p) != '=')
		{
			std::cout <<
			"SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" <<
			Paths::Config <<
			"') LINE NUMBER " <<
			p.line_count << "\n";

			assert(false);
		}

		if(attribute == "scene")
		{
			p.ParseAllWhitespace();
			p.ParseToken();
			config.initial_scene = get_parsed<std::string>(p);
		}
		else if(attribute == "camspeed")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			config.camspeed = get_parsed<float>(p);
		}
		else if(attribute == "ambient_light")
		{
			p.ParseAllWhitespace();
			p.ParseVec3();
			config.ambient_light = get_parsed<glm::vec3>(p);
		}
		else if(attribute == "ambient_intensity")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			config.ambient_intensity = get_parsed<float>(p);
		}
	}

	return config;
}

void ConfigSerializer::parse_camera_settings(Parser& p)
{
	p.ParseAllWhitespace();
	p.ParseVec3();
	scene_info->camera->position = get_parsed<glm::vec3>(p);

	p.ParseAllWhitespace();
	p.ParseVec3();
	camera_look_at(scene_info->camera, get_parsed<glm::vec3>(p), false);
}


bool ConfigSerializer::save(const ProgramConfig& config)
{
	std::ofstream writer(Paths::Config);
	if(!writer.is_open())
	{
		std::cout << "Saving config file failed.\n";
		return false;
	}

	writer << "scene = " << config.initial_scene << "\n";
	writer << "camspeed = " << config.camspeed << "\n";
	writer << "ambient_light = "
	<< config.ambient_light.x << " "
	<< config.ambient_light.y << " "
	<< config.ambient_light.z << "\n";

	writer << "ambient_intensity = " << config.ambient_intensity << "\n";

	writer.close();
	std::cout << "Config file saved succesfully.\n";

	return true;
}
