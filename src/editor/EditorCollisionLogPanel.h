#pragma once

namespace Editor
{
	// Make the UI compact because there are so many fields
	void PushStyleCompact();
	void PopStyleCompact();

	struct CollisionLogPanelContext;
	void RenderCollisionLogPanel(CollisionLogPanelContext* panel);
}
