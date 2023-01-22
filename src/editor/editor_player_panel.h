#pragma once

#include "engine/core/core.h"

// -------------
// PLAYER PANEL
// -------------

namespace Editor
{
	void render_player_panel(struct PlayerPanelContext* panel);
	void open_player_panel(Player* player);
}