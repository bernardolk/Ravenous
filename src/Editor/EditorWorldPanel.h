#pragma once

#include "engine/core/core.h"

namespace Editor
{
	struct RWorldPanelContext;

	void RenderWorldPanel(RWorldPanelContext* Panel, const RWorld* World, const EPlayer* Player);
}
