#pragma once

#include "engine/core/core.h"
#include "editor/EditorPanelContexts.h"
#include "editor/EditorUndo.h"
#include "EntityState.h"

namespace Editor
{
	enum EdToolCallback
	{
		EdToolCallback_NoCallback           = 0,
		EdToolCallback_EntityManagerSetType = 1,
	};

	struct EdToolCallbackArgs
	{
		EEntity* entity;
	};

	struct EditorContext
	{
		DeclSingleton(EditorContext)
		
		struct ImGuiStyle* im_style;

		// scene tracking
		std::string last_frame_scene;

		// undo stack
		UndoStack undo_stack;

		// deletion log
		DeletedEntityLog deletion_log;

		// panels
		SceneObjectsPanelContext scene_objects_panel;
		EntityPanelContext entity_panel;
		PlayerPanelContext player_panel;
		WorldPanelContext world_panel;
		PalettePanelContext palette_panel;
		LightsPanelContext lights_panel;
		CollisionLogPanelContext collision_log_panel;
		InputRecorderPanelContext input_recorder_panel;

		// toolbar
		bool toolbar_active = true;

		// general mode controls
		bool mouse_click = false;
		bool mouse_dragging = false;

		EEntity* selected_entity = nullptr;

		// move mode
		bool move_mode = false;
		bool scale_on_drop = false;
		u8 move_axis = 0;

		// move entity by arrows
		bool move_entity_by_arrows = false;
		vec3 move_entity_by_arrows_ref_point = vec3(0);

		// rotate entity with mouse
		bool rotate_entity_with_mouse = false;
		vec2 rotate_entity_with_mouse_mouse_coords_ref = vec2(0);

		// place mode
		bool place_mode = false;

		// move light @todo: will disappear!
		std::string selected_light_type = "";
		int selected_light = -1;

		// scale mode
		bool scale_entity_with_mouse = false;

		// measure mode
		bool measure_mode = false;
		u8 measure_axis = 0; // x,y,z == 0,1,2
		vec3 measure_from;
		bool first_point_found = false;
		bool second_point_found = false;
		float measure_to;

		// locate coordinates mode
		bool locate_coords_mode = false;
		bool locate_coords_found_point = false;
		vec3 locate_coords_position;

		// snap mode
		bool snap_mode = false;
		u8 snap_cycle = 0;
		u8 snap_axis = 1;
		bool snap_inside = false;
		EEntity* snap_reference = nullptr;
		EntityState snap_tracked_state;

		// stretch mode
		bool stretch_mode = false;

		// select entity aux tool
		bool select_entity_aux_mode = false;
		EEntity** select_entity_aux_mode_entity_slot = nullptr;
		EdToolCallback select_entity_aux_mode_callback = EdToolCallback_NoCallback;
		EdToolCallbackArgs select_entity_aux_mode_callback_args = EdToolCallbackArgs{};

		// show things 
		bool show_event_triggers = false;
		bool show_world_cells = false;
		bool show_lightbulbs = true;

		// gizmos
		EEntity* tri_axis[3];
		EEntity* tri_axis_letters[3];

		// debug options
		bool debug_ledge_detection = false;
	};
}
