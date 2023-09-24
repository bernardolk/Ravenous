#pragma once

#include "engine/core/core.h"

namespace Editor
{
	struct RWorldPanelContext;

	void RenderWorldPanel(RWorldPanelContext* panel, const RWorld* world, const EPlayer* player);
}
