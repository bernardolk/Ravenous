#pragma once
#include "engine/core/core.h"

// -------------
// LIGHTS PANEL
// -------------
namespace Editor
{
	struct LightsPanelContext;

	void open_lights_panel(std::string type = "", int index = -1, bool focus_tab = false);
	vec3 compute_direction_from_angles(float pitch, float yaw);
	void render_lights_panel(LightsPanelContext* panel, World* world);
}
