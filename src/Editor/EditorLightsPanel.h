#pragma once
#include "engine/core/core.h"

namespace Editor
{
	struct RLightsPanelContext;

	void OpenLightsPanel(string Type = "", int Index = -1, bool FocusTab = false);
	vec3 ComputeDirectionFromAngles(float Pitch, float Yaw);
	void RenderLightsPanel(RLightsPanelContext* Panel, RWorld* World);
}
