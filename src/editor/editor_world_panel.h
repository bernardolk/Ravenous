#pragma once

#include "engine/core/core.h"

namespace Editor
{
	struct WorldPanelContext;

	void RenderWorldPanel(WorldPanelContext* panel, const T_World* world, const Player* player);
}
