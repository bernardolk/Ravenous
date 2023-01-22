#pragma once

#include "engine/core/core.h"

// ---------------
// > WORLD PANEL
// ---------------

namespace Editor
{
	struct WorldPanelContext;
	
	void render_world_panel(WorldPanelContext* panel, const World* world, const Player* player);
}