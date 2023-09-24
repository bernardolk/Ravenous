#pragma once
#include "engine/core/core.h"

namespace Editor
{
	struct RLightsPanelContext;

	void OpenLightsPanel(std::string type = "", int index = -1, bool focus_tab = false);
	vec3 ComputeDirectionFromAngles(float pitch, float yaw);
	void RenderLightsPanel(RLightsPanelContext* panel, RWorld* world);
}
