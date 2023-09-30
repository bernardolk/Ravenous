#include <string>
#include <iostream>
#include <engine/core/core.h>
#include "engine/rvn.h"
#include <glm/gtx/quaternion.hpp>
#include "engine/camera/camera.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/serialization/sr_Config.h"
#include <fstream>


void ConfigSerializer::LoadGlobalConfigs()
{
	auto Parse = Parser{Paths::Config};
	auto& Config = *ProgramConfig::Get();

	while (Parse.NextLine())
	{
		Parse.ParseToken();
		const auto Attribute = GetParsed<std::string>(Parse);

		Parse.ParseAllWhitespace();
		Parse.ParseSymbol();

		if (GetParsed<char>(Parse) != '=')
		{
			std::cout <<
			"SYNTAX ERROR, MISSING '=' CHARACTER AT SCENE DESCRIPTION FILE ('" <<
			Paths::Config <<
			"') LINE NUMBER " <<
			Parse.LineCount << "\n";

			assert(false);
		}

		if (Attribute == "scene")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseToken();
			Config.InitialScene = GetParsed<std::string>(Parse);
		}
		else if (Attribute == "camspeed")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			Config.Camspeed = GetParsed<float>(Parse);
		}
		else if (Attribute == "ambient_light")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseVec3();
			Config.AmbientLight = GetParsed<glm::vec3>(Parse);
		}
		else if (Attribute == "ambient_intensity")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			Config.AmbientIntensity = GetParsed<float>(Parse);
		}
	}
}

void ConfigSerializer::ParseCameraSettings(Parser& Parse)
{
	auto* CamManager = RCameraManager::Get();
	auto* Camera = CamManager->GetCurrentCamera();

	Parse.ParseAllWhitespace();
	Parse.ParseVec3();
	Camera->Position = GetParsed<glm::vec3>(Parse);

	Parse.ParseAllWhitespace();
	Parse.ParseVec3();
	CamManager->CameraLookAt(Camera, GetParsed<glm::vec3>(Parse), false);
}


bool ConfigSerializer::Save(const ProgramConfig& Config)
{
	std::ofstream Writer(Paths::Config);
	if (!Writer.is_open())
	{
		std::cout << "Saving config file failed.\n";
		return false;
	}

	Writer << "scene = " << Config.InitialScene << "\n";
	Writer << "camspeed = " << Config.Camspeed << "\n";
	Writer << "ambient_light = "
	<< Config.AmbientLight.x << " "
	<< Config.AmbientLight.y << " "
	<< Config.AmbientLight.z << "\n";

	Writer << "ambient_intensity = " << Config.AmbientIntensity << "\n";

	Writer.close();
	std::cout << "Config file saved succesfully.\n";

	return true;
}
