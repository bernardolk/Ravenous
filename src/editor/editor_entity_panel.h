#pragma once

#include "engine/core/core.h"

// -------------
// ENTITY PANEL
// -------------
namespace Editor
{
	struct EntityPanelContext;

	void undo_selected_entity_move_changes();
	void open_entity_panel(Entity* entity);
	void check_for_asset_changes();
	void UpdateEntityControlArrows(EntityPanelContext* panel);
	void UpdateEntityRotationGizmo(EntityPanelContext* panel);
	void render_entity_control_arrows(EntityPanelContext* panel);
	void render_entity_panel(EntityPanelContext* panel, World* world);
	void entity_panel_update_entity_and_editor_context(const EntityPanelContext* panel, u32 action, World* world);
	void entity_panel_track_entity_changes(EntityPanelContext* panel);


	enum EntityPanelTrackableAction
	{
		EntityPanelTA_Position  = 1 << 0,
		EntityPanelTA_Rotation  = 1 << 1,
		EntityPanelTA_Scale     = 1 << 2,
		EntityPanelTA_Duplicate = 1 << 3,
		EntityPanelTA_Delete    = 1 << 4,
	};
}