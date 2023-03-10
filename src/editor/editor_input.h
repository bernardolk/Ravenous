#pragma once

#include "engine/core/core.h"

struct InputFlags;

namespace Editor
{
	void HandleInputFlags(InputFlags flags, World* world, Camera* camera);
}
