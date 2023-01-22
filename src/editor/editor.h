#pragma once

#include "engine/core/core.h"
#include "editor_context.h"
#include "engine/rvn.h"


namespace Editor
{
	void start_dear_imgui_frame();
	void end_dear_imgui_frame();
	
	const static std::string EDITOR_ASSETS = Paths::Project + "/assets/editor/";

	constexpr static float TRIAXIS_SCREENPOS_X = -1.80f;
	constexpr static float TRIAXIS_SCREENPOS_Y = -1.80f;


	void initialize();
	void update(Player* player, World* world, Camera* camera);
	void render(Player* player, World* world, Camera* camera);
	void terminate();

	void update_triaxis_gizmo();
	void check_selection_to_open_panel(Player* player, World* world, Camera* camera);
	bool check_selection_to_grab_entity_arrows(Camera* camera);
	bool check_selection_to_grab_entity_rotation_gizmo(Camera* camera);
	void check_selection_to_move_entity(World* world, Camera* camera);
	void check_selection_to_select_related_entity(World* world, Camera* camera);

	void render_text_overlay(Player* player, Camera* camera);
	void render_event_triggers(Camera* camera, World* world);
	void update_entity_control_arrows(EntityPanelContext* panel);
	void render_entity_control_arrows(EntityPanelContext* panel, World* world, Camera* camera);
	void render_entity_rotation_gizmo(EntityPanelContext* panel, World* world, Camera* camera);
	void update_entity_rotation_gizmo(EntityPanelContext* panel);
	void render_entity_mesh_normals(EntityPanelContext* panel);
	float _get_gizmo_scaling_factor(Entity* entity, float min, float max);
	void render_world_cells(Camera* camera, World* world);
	void render_lightbulbs(Camera* camera, World* world);
	void start_dear_imgui_frame();
	void end_dear_imgui_frame();

	inline EditorContext* GetContext() { return EditorContext::Get(); }
}
