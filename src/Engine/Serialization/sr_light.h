#pragma once

#include "engine/core/core.h"

struct Parser;

struct LightSerializer
{
	static void Parse(Parser& p);
	static void ParsePointLight(Parser& p);
	static void ParseSpotLight(Parser& p);
	static void ParseDirectionalLight(Parser& p);

	static void Save(std::ofstream& writer, const PointLight* light);
	static void Save(std::ofstream& writer, const SpotLight* light);
	static void Save(std::ofstream& writer, const DirectionalLight* light);
};
