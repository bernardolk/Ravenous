#pragma once

#include "engine/core/core.h"

struct Parser;

struct PlayerSerializer
{
	static void ParseAttribute(Parser& p);
	static void ParseOrientation(Parser& p);
	static void Save(std::ofstream& writer);
};
