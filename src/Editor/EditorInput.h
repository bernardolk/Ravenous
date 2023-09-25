#pragma once

#include "engine/core/core.h"

struct RInputFlags;

namespace Editor
{
	void HandleInputFlagsForEditorMode(RInputFlags Flags, RWorld* World);
	void HandleInputFlagsForCommonInput(RInputFlags Flags, EPlayer* & Player);
}
