#pragma once

#include "engine/core/core.h"

namespace Editor
{
	struct REntityPanelContext;

	void UndoSelectedEntityMoveChanges();
	void OpenEntityPanel(EEntity* Entity);
	void CheckForAssetChanges();
	void UpdateEntityControlArrows(REntityPanelContext* Panel);
	void UpdateEntityRotationGizmo(REntityPanelContext* Panel);
	void RenderEntityControlArrows(REntityPanelContext* Panel);
	void RenderEntityPanel(REntityPanelContext* Panel, RWorld* World);
	void EntityPanelUpdateEntityAndEditorContext(const REntityPanelContext* Panel, uint Action, RWorld* World);
	void EntityPanelTrackEntityChanges(REntityPanelContext* Panel);


	enum NEntityPanelTrackableAction
	{
		EntityPanelTA_Position  = 1 << 0,
		EntityPanelTA_Rotation  = 1 << 1,
		EntityPanelTA_Scale     = 1 << 2,
		EntityPanelTA_Duplicate = 1 << 3,
		EntityPanelTA_Delete    = 1 << 4,
	};
}
