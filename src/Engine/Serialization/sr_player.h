#pragma once

#include "engine/core/core.h"

struct Parser;

struct PlayerSerializer
{
	static void ParseAttribute(Parser& P);
	static void ParseOrientation(Parser& P);
	static void Save(std::ofstream& Writer);
};
