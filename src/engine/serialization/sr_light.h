#pragma once

#include "engine/core/core.h"

struct Parser;

struct LightSerializer
{
	static inline World* world = nullptr;

	static void parse(Parser& p);
	static void _parse_point_light(Parser& p);
	static void _parse_spot_light(Parser& p);
	static void _parse_directional_light(Parser& p);

	static void save(std::ofstream& writer, const PointLight* light);
	static void save(std::ofstream& writer, const SpotLight* light);
	static void save(std::ofstream& writer, const DirectionalLight* light);
};
