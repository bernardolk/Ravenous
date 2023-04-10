#pragma once

#include "engine/core/core.h"

constexpr static size_t MaxEntityWorldChunks = 20;
const static std::string DefaultEntityShader = "model";
const static std::string EntityShaderMarking = "color";

/** Basic data needed for lower level systems to recognize an Entity type. */
struct E_BaseEntity
{
	TypeID type_id;
	u64 id = 0;
	string name;
	bool deleted = false;
};
