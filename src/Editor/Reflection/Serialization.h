#pragma once
#include "engine/core/types.h"

struct EEntity;

namespace Serialization
{
	void SaveWorldToDisk();
	void LoadWorldFromDisk();
	EEntity* LoadEntityFromFile(RUUID ID);
}
