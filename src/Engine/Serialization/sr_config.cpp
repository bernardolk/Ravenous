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
	auto P = Parser{Paths::Config};
	auto& Config = *ProgramConfig::Get();

	while (P.NextLine())
	{
		P.ParseToken();
		const auto Attribute = GetParsed<std::string>(P);

		P.ParseAllWhitespace();
		P.ParseSymbol();

		if (GetParsed<char>(P) != '=')
		{
			std::cout <<
			"SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" <<
			Paths::Config <<
			"') LINE NUMBER " <<
			P.LineCount << "\n";

			assert(false);
		}

		if (Attribute == "scene")
		{
			P.ParseAllWhitespace();
			P.ParseToken();
			Config.InitialScene = GetParsed<std::string>(P);
		}
		else if (Attribute == "camspeed")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			Config.Camspeed = GetParsed<float>(P);
		}
		else if (Attribute == "ambient_light")
		{
			P.ParseAllWhitespace();
			P.ParseVec3();
			config.ambient_light = GetParsed<glm::vec3>(p);
		}
		else if (Attribute == "ambient_intensity")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			Config.AmbientIntensity = GetParsed<float>(P);
		}
	}
}

void ConfigSerializer::ParseCameraSettings(Parser& P)
{
	auto* CamManager = RCameraManager::Get();
	auto* Camera = CamManager->GetCurrentCamera();

	P.ParseAllWhitespace();
	P.ParseVec3();
	camera->position = GetParsed<glm::vec3>(p);

	P.ParseAllWhitespace();
	P.ParseVec3();
	cam_manager->CameraLookAt(camera, GetParsed<glm::vec3>(p), false);
}


bool ConfigSerializer::Save(const ProgramConfig& Config)
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
