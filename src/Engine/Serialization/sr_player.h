#pragma once

#include "engine/core/core.h"

struct Parser;

struct PlayerSerializer
{
	static void ParseAttribute(Parser& Parse);
	static void ParseOrientation(Parser& Parse);
	static void Save(std::ofstream& Writer);
};
