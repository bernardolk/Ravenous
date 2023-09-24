#pragma once

#include "engine/core/core.h"

namespace Editor
{
	struct REntityPanelContext;

	void UndoSelectedEntityMoveChanges();
	void OpenEntityPanel(EEntity* entity);
	void CheckForAssetChanges();
	void UpdateEntityControlArrows(REntityPanelContext* panel);
	void UpdateEntityRotationGizmo(REntityPanelContext* panel);
	void RenderEntityControlArrows(REntityPanelContext* panel);
	void RenderEntityPanel(REntityPanelContext* panel, RWorld* world);
	void EntityPanelUpdateEntityAndEditorContext(const REntityPanelContext* panel, uint action, RWorld* world);
	void EntityPanelTrackEntityChanges(REntityPanelContext* panel);


	enum EntityPanelTrackableAction
	{
		EntityPanelTA_Position  = 1 << 0,
		EntityPanelTA_Rotation  = 1 << 1,
		EntityPanelTA_Scale     = 1 << 2,
		EntityPanelTA_Duplicate = 1 << 3,
		EntityPanelTA_Delete    = 1 << 4,
	};
}
