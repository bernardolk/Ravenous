#pragma once

#include "engine/core/core.h"

// --------------
// PALETTE PANEL
// --------------
namespace Editor
{
	struct PalettePanelContext;
	
	void render_palette_panel(PalettePanelContext* panel);
	void initialize_palette(PalettePanelContext* panel);
}
