#pragma once

#include "engine/core/core.h"

struct RInputFlags;

namespace Editor
{
	void HandleInputFlagsForEditorMode(RInputFlags flags, RWorld* world);
	void HandleInputFlagsForCommonInput(RInputFlags flags, EPlayer* & player);
}
