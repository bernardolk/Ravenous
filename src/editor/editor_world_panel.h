#pragma once

#include "engine/core/core.h"

namespace Editor
{
	struct WorldPanelContext;

	void RenderWorldPanel(WorldPanelContext* panel, const World* world, const Player* player);
}
