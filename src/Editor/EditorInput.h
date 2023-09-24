#pragma once

#include "engine/core/core.h"

struct InputFlags;

namespace Editor
{
	void HandleInputFlagsForEditorMode(InputFlags flags, RWorld* world);
	void HandleInputFlagsForCommonInput(InputFlags flags, EPlayer* & player);
}
