#include <string>
#include <iostream>
#include <engine/core/core.h>
#include "engine/rvn.h"
#include <glm/gtx/quaternion.hpp>
#include "engine/camera/camera.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_config.h"
#include <fstream>


void ConfigSerializer::LoadGlobalConfigs()
{
	auto p = Parser{Paths::Config};
	auto& config = *ProgramConfig::Get();

	while (p.NextLine())
	{
		p.ParseToken();
		const auto attribute = GetParsed<std::string>(p);

		p.ParseAllWhitespace();
		p.ParseSymbol();

		if (GetParsed<char>(p) != '=')
		{
			std::cout <<
			"SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" <<
			Paths::Config <<
			"') LINE NUMBER " <<
			p.line_count << "\n";

			assert(false);
		}

		if (attribute == "scene")
		{
			p.ParseAllWhitespace();
			p.ParseToken();
			config.initial_scene = GetParsed<std::string>(p);
		}
		else if (attribute == "camspeed")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			config.camspeed = GetParsed<float>(p);
		}
		else if (attribute == "ambient_light")
		{
			p.ParseAllWhitespace();
			p.ParseVec3();
			config.ambient_light = GetParsed<glm::vec3>(p);
		}
		else if (attribute == "ambient_intensity")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			config.ambient_intensity = GetParsed<float>(p);
		}
	}
}

void ConfigSerializer::ParseCameraSettings(Parser& p)
{
	auto* cam_manager = CameraManager::Get();
	auto* camera = cam_manager->GetCurrentCamera();

	p.ParseAllWhitespace();
	p.ParseVec3();
	camera->position = GetParsed<glm::vec3>(p);

	p.ParseAllWhitespace();
	p.ParseVec3();
	cam_manager->CameraLookAt(camera, GetParsed<glm::vec3>(p), false);
}


bool ConfigSerializer::Save(const ProgramConfig& config)
{
	std::ofstream writer(Paths::Config);
	if (!writer.is_open())
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
