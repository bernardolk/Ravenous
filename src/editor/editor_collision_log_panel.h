#pragma once

#include "engine/core/core.h"

// --------------------
// COLLISION LOG PANEL
// --------------------

namespace Editor
{
	// Make the UI compact because there are so many fields
	void PushStyleCompact();
	void PopStyleCompact();

	struct CollisionLogPanelContext;
	void render_collision_log_panel(CollisionLogPanelContext* panel);
}

