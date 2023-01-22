#pragma once

#include "engine/core/core.h"

struct InputFlags;

namespace Editor
{
	void handle_input_flags(InputFlags flags, World* world, Camera* camera);
}
