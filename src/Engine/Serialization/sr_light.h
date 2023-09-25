#pragma once

#include "engine/core/core.h"

struct Parser;

struct LightSerializer
{
	static void Parse(Parser& P);
	static void ParsePointLight(Parser& P);
	static void ParseSpotLight(Parser& P);
	static void ParseDirectionalLight(Parser& P);

	static void Save(std::ofstream& Writer, const PointLight* Light);
	static void Save(std::ofstream& Writer, const SpotLight* Light);
	static void Save(std::ofstream& Writer, const DirectionalLight* Light);
};
