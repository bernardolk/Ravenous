#pragma once

#include "engine/core/core.h"
#include "editor/editor_context.h"
#include "engine/collision/raycast.h"

namespace Editor
{
	void deactivate_editor_modes();
	bool check_modes_are_active();
	void editor_erase_entity(Entity* entity);
	void editor_erase_light(int index, std::string type, World* world);
	void unhide_entities(const World* world);

	// ----------
	// SNAP TOOL
	// ----------
	void activate_snap_mode(Entity* entity);
	void snap_entity_to_reference(Entity* entity);
	void check_selection_to_snap();
	void snap_commit();

	// -------------
	// STRETCH TOOL
	// -------------
	void activate_stretch_mode(Entity* entity);
	void stretch_commit();
	auto get_scale_and_position_change(Entity* entity, float old_pos, float new_pos, float n);
	void stretch_entity_to_reference(Entity* entity);
	void check_selection_to_stretch();


	// -------------
	// MEASURE TOOL
	// -------------
	void activate_measure_mode(u8 axis);
	void check_selection_to_measure(const World* world);

	// ------------------------
	// LOCATE COORDINATES MODE
	// ------------------------
	void activate_locate_coords_mode();
	void check_selection_to_locate_coords(const World* world);

	// -------------
	// > MOVE TOOLS 
	// -------------
	void place_entity(World* world);

	RaycastTest test_ray_against_entity_support_plane(u16 move_axis, Entity* entity);

	// --------------
	// >> PLACE MODE
	// --------------
	void activate_place_mode(Entity* entity);

	void select_entity_placing_with_mouse_move(Entity* entity, const World* world);


	// -------------
	// >> MOVE MODE
	// -------------
	void activate_move_mode(Entity* entity);

	void move_entity_with_mouse(Entity* entity);


	// -------------------------
	// >> MOVE ENTITY BY ARROWS
	// -------------------------
	void activate_move_entity_by_arrow(u8 move_axis);
	void move_entity_by_arrows(Entity* entity);



	// ----------------
	// MOVE LIGHT TOOL
	// ----------------
	// @todo: This will DISAPPEAR after lights become entities!
	//       We need to provide entity rights to lights too! revolution now!

	void move_light_with_mouse(std::string type, int index, World* world);
	void activate_move_light_mode(std::string type, int index);
	void place_light(std::string type, int index);
	void place_light();
	void open_lights_panel(std::string type, int index, bool focus_tab); //fwd



	// ---------------------
	// > ROTATE ENTITY TOOL
	// ---------------------
	void activate_rotate_entity_with_mouse(u8 move_axis);
	float mouse_offset_to_angular_offset(float mouse_offset);
	void rotate_entity_with_mouse(Entity* entity);


	// ------------------
	// SCALE ENTITY TOOL
	// ------------------
	void scale_entity_with_mouse(Entity* entity);

	// -----------------------
	// SELECT ENTITY AUX TOOL
	// -----------------------
	// used in entity panel to select other entity to attribute 1 to 1 relationships
	void activate_select_entity_aux_tool(
		Entity** entity_slot,
		Editor::EdToolCallback callback = Editor::EdToolCallback_NoCallback,
		Editor::EdToolCallbackArgs args = Editor::EdToolCallbackArgs{}
	);

	// -------------
	// MISCELANEOUS
	// -------------
	// void check_for_asset_changes();
	void render_aabb_boundaries(Entity* entity);
}
