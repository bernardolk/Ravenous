#pragma once

#include "engine/core/core.h"

namespace Editor
{
	struct EntityPanelContext;

	void UndoSelectedEntityMoveChanges();
	void OpenEntityPanel(Entity* entity);
	void CheckForAssetChanges();
	void UpdateEntityControlArrows(EntityPanelContext* panel);
	void UpdateEntityRotationGizmo(EntityPanelContext* panel);
	void RenderEntityControlArrows(EntityPanelContext* panel);
	void RenderEntityPanel(EntityPanelContext* panel, World* world);
	void EntityPanelUpdateEntityAndEditorContext(const EntityPanelContext* panel, u32 action, World* world);
	void EntityPanelTrackEntityChanges(EntityPanelContext* panel);


	enum EntityPanelTrackableAction
	{
		EntityPanelTA_Position  = 1 << 0,
		EntityPanelTA_Rotation  = 1 << 1,
		EntityPanelTA_Scale     = 1 << 2,
		EntityPanelTA_Duplicate = 1 << 3,
		EntityPanelTA_Delete    = 1 << 4,
	};
}
