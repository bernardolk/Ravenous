#pragma once

#include "engine/core/core.h"

struct InputFlags;

namespace Editor
{
	void HandleInputFlagsForEditorMode(InputFlags flags, World* world);
	void HandleInputFlagsForCommonInput(InputFlags flags, Player* & player);
}
