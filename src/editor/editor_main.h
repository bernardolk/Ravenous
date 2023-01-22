#pragma once

#include "engine/core/core.h"
#include "editor_colors.h"

namespace Editor
{

	const static std::string EDITOR_ASSETS = Paths::Project + "/assets/editor/";

	constexpr static float TRIAXIS_SCREENPOS_X = -1.80f;
	constexpr static float TRIAXIS_SCREENPOS_Y = -1.80f;

#include <editor/editor_undo.h>
#include <editor/editor_panel_contexts.h>

	enum EdToolCallback
	{
		EdToolCallback_NoCallback           = 0,
		EdToolCallback_EntityManagerSetType = 1,
	};

	struct EdToolCallbackArgs
	{
		Entity* entity;
		union
		{
			EntityType entity_type;
		};
	};

	struct EditorContext
	{
		ImGuiStyle* imStyle;

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

		Entity* selected_entity = nullptr;

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
		Entity* snap_reference = nullptr;
		EntityState snap_tracked_state;

		// stretch mode
		bool stretch_mode = false;

		// select entity aux tool
		bool select_entity_aux_mode = false;
		Entity** select_entity_aux_mode_entity_slot = nullptr;
		EdToolCallback select_entity_aux_mode_callback = EdToolCallback_NoCallback;
		EdToolCallbackArgs select_entity_aux_mode_callback_args = EdToolCallbackArgs{};

		// show things 
		bool show_event_triggers = false;
		bool show_world_cells = false;
		bool show_lightbulbs = true;

		// gizmos
		Entity* tri_axis[3];
		Entity* tri_axis_letters[3];

		// debug options
		bool debug_ledge_detection = false;

	} EdContext;


	void initialize();
	void update(Player* player);
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



#include <editor/editor_tools.h>
#include <editor/editor_toolbar.h>
#include <editor/editor_entity_panel.h>
#include <editor/editor_player_panel.h>
#include <editor/editor_world_panel.h>
#include <editor/editor_input.h>
#include <editor/editor_palette_panel.h>
#include <editor/editor_lights_panel.h>
#include <editor/editor_collision_log_panel.h>
#include <editor/editor_input_recorder_panel.h>
#include <editor/editor_scene_objects_panel.h>

//------------------
// > UPDATE EDITOR
//------------------

	void update(Player* player, World* world, Camera* camera)
	{
		auto* GSI = GlobalSceneInfo::Get();

		if(EdContext.last_frame_scene != GSI->scene_name)
		{
			EdContext.entity_panel.active = false;
			EdContext.world_panel.active = false;
		}

		EdContext.last_frame_scene = GSI->scene_name;

		// check for asset changes
		// check_for_asset_changes();
		update_triaxis_gizmo();

		// ENTITY PANEL
		if(!EdContext.entity_panel.active)
		{
			EdContext.entity_panel.rename_buffer[0] = 0;
			EdContext.snap_mode = false;
			EdContext.stretch_mode = false;
			EdContext.snap_reference = nullptr;
		}

		// unselect lights when not panel is not active
		if(!EdContext.lights_panel.active)
		{
			EdContext.lights_panel.selected_light = -1;
			EdContext.lights_panel.selected_light_type = "";
		}
		else if(
			EdContext.lights_panel.selected_light != -1 &&
			EdContext.lights_panel.selected_light_type != ""
		)
		{
			EdContext.show_lightbulbs = true;
		}


		// set editor mode values to initial if not active
		if(!EdContext.measure_mode)
		{
			EdContext.first_point_found = false;
			EdContext.second_point_found = false;
		}
		if(!EdContext.snap_mode)
		{
			EdContext.snap_cycle = 0;
			EdContext.snap_axis = 1;
			EdContext.snap_reference = nullptr;
		}

		// respond to mouse if necessary
		if(EdContext.move_mode)
		{
			if(EdContext.mouse_click)
			{
				if(EdContext.selected_light > -1)
					place_light();
				else
					place_entity(world);
			}
			else
			{
				if(EdContext.selected_light > -1)
					move_light_with_mouse(EdContext.selected_light_type, EdContext.selected_light, world);
				else
					move_entity_with_mouse(EdContext.selected_entity);
			}
		}

		if(EdContext.select_entity_aux_mode)
		{
			if(EdContext.mouse_click)
			{
				check_selection_to_select_related_entity(world, camera);
			}
		}

		if(EdContext.move_entity_by_arrows)
		{
			if(EdContext.mouse_dragging)
				move_entity_by_arrows(EdContext.selected_entity);
				// the below condition is to prevent from deactivating too early
			else if(!EdContext.mouse_click)
				place_entity(world);
		}

		if(EdContext.rotate_entity_with_mouse)
		{
			if(EdContext.mouse_dragging)
				rotate_entity_with_mouse(EdContext.selected_entity);
				// the below condition is to prevent from deactivating too early
			else if(!EdContext.mouse_click)
				place_entity(world);
		}


		if(EdContext.place_mode)
		{
			if(EdContext.mouse_click)
				place_entity(world);
			else
				select_entity_placing_with_mouse_move(EdContext.selected_entity, world);
		}

		if(EdContext.scale_entity_with_mouse)
		{
			scale_entity_with_mouse(EdContext.selected_entity);
		}

		// resets mouse click event
		EdContext.mouse_click = false;

		// check for debug flags
		if(EdContext.debug_ledge_detection)
		{
			CL_perform_ledge_detection(player, world);
		}
	}

	void update_triaxis_gizmo()
	{
		for(int i = 0; i < 3; i++)
		{
			auto entity = EdContext.tri_axis[i];
			glm::mat4 model = Mat4Identity;
			model = rotate(model, glm::radians(entity->rotation.x), vec3(1.0f, 0.0f, 0.0f));
			model = rotate(model, glm::radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
			model = rotate(model, glm::radians(entity->rotation.z), vec3(0.0f, 0.0f, 1.0f));
			model = scale(model, entity->scale);
			entity->mat_model = model;
		}
	}


//---------------------
// > RENDER EDITOR UI
//---------------------

	void render(Player* player, World* world, Camera* camera)
	{
		// render world objs if toggled
		if(EdContext.show_event_triggers)
		{
			render_event_triggers(camera, world);
		}

		if(EdContext.show_world_cells)
		{
			render_world_cells(camera, world);
		}

		if(EdContext.show_lightbulbs)
		{
			render_lightbulbs(camera, world);
		}

		// render triaxis
		auto triaxis_view = lookAt(vec3(0.0f), camera->front, -1.0f * camera->up);
		float displacement_x[3] = {0.3f, 0.0f, 0.0f};
		float displacement_y[3] = {0.0f, 0.3f, 0.0f};
		for(int i = 0; i < 3; i++)
		{
			// ref. axis
			auto axis = EdContext.tri_axis[i];
			axis->shader->Use();
			axis->shader->SetMatrix4("model", axis->mat_model);
			axis->shader->SetMatrix4("view", triaxis_view);
			axis->shader->SetFloat2("screenPos", TRIAXIS_SCREENPOS_X, TRIAXIS_SCREENPOS_Y);
			RenderEntity(axis);
		}

		// Entity panel special render calls
		if(EdContext.entity_panel.active)
		{
			// Render glowing pink wireframe on top of selected entity
			{
				// update
				auto state = get_entity_state(EdContext.selected_entity);
				auto model = mat_model_from_entity_state(state);

				// compute color intensity based on time
				float time_value = glfwGetTime();
				float intensity = sin(time_value) * 2;
				if(intensity < 0)
					intensity *= -1.0;
				intensity += 1.0;

				// render
				auto glowing_line = ShaderCatalogue.find("color")->second;
				glowing_line->Use();
				glowing_line->SetMatrix4("model", model);
				glowing_line->SetFloat3("color", intensity * 0.890, intensity * 0.168, intensity * 0.6);
				glowing_line->SetFloat("opacity", 1);
				render_mesh(EdContext.selected_entity->mesh, RenderOptions{true, false, 3});
			}

			// Render glowing yellow wireframe on top of an arbitrary related entity
			if(EdContext.entity_panel.show_related_entity)
			{
				// update
				auto state = get_entity_state(EdContext.entity_panel.related_entity);
				auto model = mat_model_from_entity_state(state);

				// compute color intensity based on time
				float time_value = glfwGetTime();
				float intensity = sin(time_value) * 2;
				if(intensity < 0)
					intensity *= -1.0;
				intensity += 1.0;

				// render
				auto glowing_line = ShaderCatalogue.find("color")->second;
				glowing_line->Use();
				glowing_line->SetMatrix4("model", model);
				glowing_line->SetFloat3("color", intensity * 0.941, intensity * 0.776, intensity * 0);
				glowing_line->SetFloat("opacity", 1);
				render_mesh(EdContext.entity_panel.related_entity->mesh, RenderOptions{true, false, 3});
			}
		}

		// render glowing wireframe on top of snap reference entity
		if(EdContext.snap_mode && EdContext.snap_reference != nullptr)
		{
			// update
			auto state = get_entity_state(EdContext.snap_reference);
			auto model = mat_model_from_entity_state(state);

			// compute color intensity based on time
			float time_value = glfwGetTime();
			float intensity = sin(time_value) * 2;
			if(intensity < 0)
				intensity *= -1.0;
			intensity += 1.0;

			// render
			auto glowing_line = ShaderCatalogue.find("color")->second;
			glowing_line->Use();
			glowing_line->SetMatrix4("model", model);
			glowing_line->SetFloat3("color", intensity * 0.952, intensity * 0.843, intensity * 0.105);
			glowing_line->SetFloat("opacity", 1);
			render_mesh(EdContext.snap_reference->mesh, RenderOptions{true, false, 3});
		}

		// --------------
		// render panels
		// --------------
		if(EdContext.scene_objects_panel.active)
			render_scene_objects_panel(world, &EdContext.scene_objects_panel);

		if(EdContext.world_panel.active)
			render_world_panel(&EdContext.world_panel, world, player);

		if(EdContext.entity_panel.active)
		{
			auto& panel = EdContext.entity_panel;

			render_entity_panel(&panel, world);
			render_entity_control_arrows(&panel, world, camera);
			render_entity_rotation_gizmo(&panel, world, camera);

			if(panel.show_normals)
				render_entity_mesh_normals(&panel);
			// @TODO: Some bug being caused in this call
			//if(panel.show_collider)
			//   ImDraw::add_mesh(IMHASH, &panel.entity->collider, COLOR_PURPLE_1, 0);
			if(panel.show_bounding_box)
			{
				auto aabb = GeometryCatalogue.find("aabb")->second;
				auto [pos, scale] = panel.entity->bounding_box.GetPosAndScale();
				ImDraw::AddMesh(IMHASH, aabb, pos, vec3(0), scale, COLOR_PINK_1, 0);
			}
		}

		if(EdContext.player_panel.active)
		{
			render_player_panel(&EdContext.player_panel);
		}

		if(EdContext.palette_panel.active)
			render_palette_panel(&EdContext.palette_panel);

		if(EdContext.lights_panel.active)
			render_lights_panel(&EdContext.lights_panel, world);

		if(EdContext.input_recorder_panel.active)
			render_input_recorder_panel(&EdContext.input_recorder_panel);

		if(EdContext.collision_log_panel.active)
			render_collision_log_panel(&EdContext.collision_log_panel);

		// -----------------------
		// render gizmos inscreen
		// -----------------------
		if(EdContext.measure_mode && EdContext.first_point_found && EdContext.second_point_found)
		{
			auto render_opts = RenderOptions();
			render_opts.always_on_top = true;
			render_opts.line_width = 2.0;
			render_opts.color = ED_RED;

			vec3 second_point;
			if(EdContext.measure_axis == 0)
				second_point = vec3(EdContext.measure_to, EdContext.measure_from.y, EdContext.measure_from.z);
			if(EdContext.measure_axis == 1)
				second_point = vec3(EdContext.measure_from.x, EdContext.measure_to, EdContext.measure_from.z);
			if(EdContext.measure_axis == 2)
				second_point = vec3(EdContext.measure_from.x, EdContext.measure_from.y, EdContext.measure_to);

			ImDraw::Add(
				IMHASH,
				std::vector<Vertex>{
				Vertex{EdContext.measure_from},
				Vertex{second_point}
				},
				GL_LINE_LOOP,
				render_opts
			);
		}

		if(EdContext.locate_coords_mode && EdContext.locate_coords_found_point)
		{
			ImDraw::AddPoint(IMHASH, EdContext.locate_coords_position, 2.0);
		}

		render_toolbar(world);

		render_text_overlay(player, camera);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void terminate()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}


	void initialize()
	{
		ImGui::CreateContext();
		auto& io = ImGui::GetIO();
		ImGui_ImplGlfw_InitForOpenGL(GlobalDisplayConfig::GetWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 330");

		ImGui::StyleColorsDark();
		EdContext.imStyle = &ImGui::GetStyle();
		EdContext.imStyle->WindowRounding = 1.0f;

		// load tri axis gizmo
		const auto axis_mesh = load_wavefront_obj_as_mesh(Paths::Models, "axis");

		const auto x_axis = new Entity();
		const auto y_axis = new Entity();
		const auto z_axis = new Entity();

		x_axis->mesh = axis_mesh;
		y_axis->mesh = axis_mesh;
		z_axis->mesh = axis_mesh;

		const auto blue_tex = load_texture_from_file("blue.jpg", Paths::Textures);
		const auto green_tex = load_texture_from_file("green.jpg", Paths::Textures);
		const auto red_tex = load_texture_from_file("red.jpg", Paths::Textures);

		x_axis->textures.push_back(Texture{red_tex, "texture_diffuse", "red.jpg", "red axis"});
		y_axis->textures.push_back(Texture{green_tex, "texture_diffuse", "green.jpg", "green axis"});
		z_axis->textures.push_back(Texture{blue_tex, "texture_diffuse", "blue.jpg", "blue axis"});

		const auto shader = ShaderCatalogue.find("ortho_gui")->second;
		x_axis->shader = shader;
		x_axis->scale = vec3{0.1, 0.1, 0.1};
		x_axis->rotation = vec3{90, 0, 90};

		y_axis->shader = shader;
		y_axis->scale = vec3{0.1, 0.1, 0.1};
		y_axis->rotation = vec3{180, 0, 0};

		z_axis->shader = shader;
		z_axis->scale = vec3{0.1, 0.1, 0.1};
		z_axis->rotation = vec3{90, 0, 180};

		EdContext.tri_axis[0] = x_axis;
		EdContext.tri_axis[1] = y_axis;
		EdContext.tri_axis[2] = z_axis;


		// load entity panel axis arrows
		// @todo: refactor this to use the entity_manager
		const auto x_arrow = new Entity();
		const auto y_arrow = new Entity();
		const auto z_arrow = new Entity();

		x_arrow->mesh = axis_mesh;
		y_arrow->mesh = axis_mesh;
		z_arrow->mesh = axis_mesh;

		const auto arrow_shader = ShaderCatalogue.find("ed_entity_arrow_shader")->second;
		x_arrow->shader = arrow_shader;
		x_arrow->scale = vec3(0.5, 0.5, 0.5);
		x_arrow->rotation = vec3(0);

		y_arrow->shader = arrow_shader;
		y_arrow->scale = vec3(0.5, 0.5, 0.5);
		y_arrow->rotation = vec3(0);

		z_arrow->shader = arrow_shader;
		z_arrow->scale = vec3(0.5, 0.5, 0.5);
		z_arrow->rotation = vec3(0);

		x_arrow->textures.push_back(Texture{red_tex, "texture_diffuse", "red.jpg", "red axis"});
		y_arrow->textures.push_back(Texture{green_tex, "texture_diffuse", "green.jpg", "green axis"});
		z_arrow->textures.push_back(Texture{blue_tex, "texture_diffuse", "blue.jpg", "blue axis"});

		// CollisionMesh
		const auto arrow_collider = new CollisionMesh();

		For(axis_mesh->vertices.size())
			arrow_collider->vertices.push_back(axis_mesh->vertices[i].position);

		For(axis_mesh->indices.size())
			arrow_collider->indices.push_back(axis_mesh->indices[i]);

		x_arrow->collision_mesh = arrow_collider;
		x_arrow->collider = *arrow_collider;

		y_arrow->collision_mesh = arrow_collider;
		y_arrow->collider = *arrow_collider;

		z_arrow->collision_mesh = arrow_collider;
		z_arrow->collider = *arrow_collider;

		EdContext.entity_panel.x_arrow = x_arrow;
		EdContext.entity_panel.y_arrow = y_arrow;
		EdContext.entity_panel.z_arrow = z_arrow;

		auto* EM = EntityManager::Get();
		// creates entity rotation gizmos
		EdContext.entity_panel.rotation_gizmo_x = EM->CreateEditorEntity({
		.name = "rotation_gizmo_x",
		.mesh = "rotation_gizmo",
		.shader = "ed_entity_arrow_shader",
		.texture = "red",
		.collision_mesh = "rotation_gizmo_collision"});

		EdContext.entity_panel.rotation_gizmo_y = EM->CreateEditorEntity({
		.name = "rotation_gizmo_y",
		.mesh = "rotation_gizmo",
		.shader = "ed_entity_arrow_shader",
		.texture = "green",
		.collision_mesh = "rotation_gizmo_collision"});

		EdContext.entity_panel.rotation_gizmo_z = EM->CreateEditorEntity({
		.name = "rotation_gizmo_z",
		.mesh = "rotation_gizmo",
		.shader = "ed_entity_arrow_shader",
		.texture = "blue",
		.collision_mesh = "rotation_gizmo_collision"});


		// palette panel
		initialize_palette(&EdContext.palette_panel);

		auto* GSI = GlobalSceneInfo::Get();
		EdContext.last_frame_scene = GSI->scene_name;
	}


	void render_text_overlay(Player* player, Camera* camera)
	{
		float GUI_y = GlobalDisplayConfig::viewport_height - 60;
		float SCREEN_HEIGHT = GlobalDisplayConfig::viewport_height;

		std::string font = "consola18";
		std::string font_center = "swanseait38";
		std::string font_center_small = "swanseait20";
		float centered_text_height = SCREEN_HEIGHT - 120;
		float centered_text_height_small = centered_text_height - 40;
		auto tool_text_color_yellow = vec3(0.8, 0.8, 0.2);
		auto tool_text_color_green = vec3(0.6, 1.0, 0.3);


		// CAMERA POSITION
		std::string cam_p[3]{
		FormatFloatTostr(camera->position.x, 2),
		FormatFloatTostr(camera->position.y, 2),
		FormatFloatTostr(camera->position.z, 2),
		};
		std::string camera_position = "camera:   x: " + cam_p[0] + " y:" + cam_p[1] + " z:" + cam_p[2];
		render_text(font, 235, 45, camera_position);


		// PLAYER POSITION
		vec3 p_feet = player->Feet();
		std::string player_p[3]{
		FormatFloatTostr(p_feet.x, 1),
		FormatFloatTostr(p_feet.y, 1),
		FormatFloatTostr(p_feet.z, 1),
		};
		std::string player_pos = "player:   x: " + player_p[0] + " y: " + player_p[1] + " z: " + player_p[2];
		render_text(font, 235, 70, player_pos);


		// PLAYER LIVES
		std::string lives = std::to_string(player->lives);
		render_text(
			font,
			GlobalDisplayConfig::viewport_width - 400,
			90,
			player->lives == 2 ? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1},
			lives
		);


		// PLAYER STATE
		auto player_state_text_color = vec3(0, 0, 0);
		std::string player_state_text;
		switch(player->player_state)
		{
			case PLAYER_STATE_STANDING: player_state_text = "PLAYER STANDING";
				break;
			case PLAYER_STATE_FALLING: player_state_text = "PLAYER FALLING";
				break;
			case PLAYER_STATE_FALLING_FROM_EDGE: player_state_text = "PLAYER FALLING FROM EDGE";
				break;
			case PLAYER_STATE_JUMPING: player_state_text = "PLAYER JUMPING";
				break;
			case PLAYER_STATE_SLIDING: player_state_text = "PLAYER SLIDING";
				break;
			case PLAYER_STATE_SLIDE_FALLING: player_state_text = "PLAYER SLIDE FALLING";
				break;
			case PLAYER_STATE_EVICTED_FROM_SLOPE: player_state_text = "PLAYER EVICTED FROM SLOPE";
				break;
		}
		render_text("consola18", GlobalDisplayConfig::viewport_width - 400, 30, player_state_text_color, player_state_text);


		std::string player_floor = "player floor: ";
		if(player->standing_entity_ptr != nullptr)
			player_floor += player->standing_entity_ptr->name;
		render_text(GlobalDisplayConfig::viewport_width - 400, 60, player_floor);

		std::string p_grab = "grabbing: ";
		if(player->grabbing_entity != nullptr)
			p_grab += player->grabbing_entity->name;
		render_text(GlobalDisplayConfig::viewport_width - 400, 45, p_grab);


		// FPS
		std::string fps = std::to_string(Rvn::frame.fps);
		std::string fps_gui = "FPS: " + fps;
		render_text(font, GlobalDisplayConfig::viewport_width - 110, 40, fps_gui);


		// EDITOR TOOLS INDICATORS

		// ----------
		// SNAP MODE
		// ----------
		if(EdContext.snap_mode)
		{
			std::string snap_cycle;
			switch(EdContext.snap_cycle)
			{
				case 0: snap_cycle = "top";
					break;
				case 1: snap_cycle = "mid";
					break;
				case 2: snap_cycle = "bottom";
					break;
			}

			std::string snap_axis;
			switch(EdContext.snap_axis)
			{
				case 0: snap_axis = "X";
					break;
				case 1: snap_axis = "Y";
					break;
				case 2: snap_axis = "Z";
					break;
			}

			// if position is changed and not commited, render text yellow
			vec3 snap_mode_subtext_color;
			if(EdContext.snap_reference == nullptr)
				snap_mode_subtext_color = tool_text_color_yellow;
			else
			{
				auto state = EdContext.undo_stack.Check();
				if(state.entity != nullptr && state.position != EdContext.entity_panel.entity->position)
					snap_mode_subtext_color = tool_text_color_yellow;
				else
					snap_mode_subtext_color = tool_text_color_green;
			}

			// selects text based on situation of snap tool
			std::string sub_text;
			if(EdContext.snap_reference == nullptr)
				sub_text = "select another entity to snap to.";
			else
				sub_text = "press Enter to commit position. x/y/z to change axis.";

			render_text(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				tool_text_color_yellow,
				true,
				"SNAP MODE (" + snap_axis + "-" + snap_cycle + ")"
			);

			render_text(
				font_center_small,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height_small,
				snap_mode_subtext_color,
				true,
				sub_text
			);
		}

		// -------------
		// MEASURE MODE
		// -------------
		if(EdContext.measure_mode)
		{
			std::string axis =
			EdContext.measure_axis == 0 ? "x" :
			EdContext.measure_axis == 1 ? "y" :
			"z";

			render_text(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"MEASURE MODE (" + axis + ")"
			);

			if(EdContext.second_point_found)
			{
				float dist_ref =
				EdContext.measure_axis == 0 ? EdContext.measure_from.x :
				EdContext.measure_axis == 1 ? EdContext.measure_from.y :
				EdContext.measure_from.z;

				render_text(
					font_center,
					GlobalDisplayConfig::viewport_width / 2,
					centered_text_height_small,
					vec3(0.8, 0.8, 0.2),
					true,
					"(" + FormatFloatTostr(abs(EdContext.measure_to - dist_ref), 2) + " m)"
				);
			}
		}

		// ----------
		// MOVE MODE
		// ----------
		if(EdContext.move_mode)
		{
			std::string move_axis;
			switch(EdContext.move_axis)
			{
				case 0: move_axis = "XZ";
					break;
				case 1: move_axis = "X";
					break;
				case 2: move_axis = "Y";
					break;
				case 3: move_axis = "Z";
					break;
			}

			render_text(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"MOVE MODE (" + move_axis + ")"
			);

			render_text(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height_small,
				vec3(0.8, 0.8, 0.2),
				true,
				"press M to alternate between move and place modes"
			);
		}

		// ----------
		// PLACE MODE
		// ----------
		if(EdContext.place_mode)
		{
			render_text(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"PLACE MODE"
			);

			render_text(
				font_center_small,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height_small,
				vec3(0.8, 0.8, 0.2),
				true,
				"press M to alternate between move and place modes"
			);
		}

		// -------------------
		// LOCATE COORDS MODE
		// -------------------
		if(EdContext.locate_coords_mode)
		{
			render_text(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"LOCATE COORDS MODE"
			);

			std::string locate_coords_subtext;
			if(!EdContext.locate_coords_found_point)
			{
				locate_coords_subtext = "Please select a world position to get coordinates.";
			}
			else
			{
				locate_coords_subtext =
				"(x: " + FormatFloatTostr(EdContext.locate_coords_position[0], 2) +
				", y: " + FormatFloatTostr(EdContext.locate_coords_position[1], 2) +
				", z: " + FormatFloatTostr(EdContext.locate_coords_position[2], 2) + ")";
			}

			render_text(
				font_center_small,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height - 40,
				tool_text_color_green,
				true,
				locate_coords_subtext
			);
		}

		// -------------
		// STRETCH MODE
		// -------------
		if(EdContext.stretch_mode)
		{
			render_text(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"STRETCH MODE"
			);
		}

		// --------------------------
		// ENTITY SELECTION AUX MODE
		// --------------------------
		if(EdContext.select_entity_aux_mode)
		{
			render_text(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"SELECT RELATED ENTITY"
			);
		}
	}


	void render_event_triggers(Camera* camera, World* world)
	{
		if(world->interactables.size() == 0)
			return;

		auto find = ShaderCatalogue.find("color");
		auto shader = find->second;

		shader->Use();
		shader->SetMatrix4("view", camera->mat_view);
		shader->SetMatrix4("projection", camera->mat_projection);

		for(int i = 0; i < world->interactables.size(); i++)
		{
			auto checkpoint = world->interactables[i];
			shader->SetMatrix4("model", checkpoint->trigger_mat_model);
			shader->SetFloat3("color", 0.5, 0.5, 0.3);
			shader->SetFloat("opacity", 0.6);
			render_mesh(checkpoint->trigger, RenderOptions{});
		}
	}


	void render_world_cells(Camera* camera, World* world)
	{
		auto shader = ShaderCatalogue.find("color")->second;
		auto cell_mesh = GeometryCatalogue.find("aabb")->second;

		for(int i = 0; i < world->cells_in_use_count; i++)
		{
			RenderOptions opts;
			opts.wireframe = true;

			auto cell = world->cells_in_use[i];

			vec3 color;
			if(EdContext.world_panel.cell_coords.x == cell->i &&
				EdContext.world_panel.cell_coords.y == cell->j &&
				EdContext.world_panel.cell_coords.z == cell->k)
			{
				opts.line_width = 1.5;
				color = vec3(0.8, 0.4, 0.2);
			}
			else if((cell->i == WCellsNumX || cell->i == 0) ||
				(cell->j == WCellsNumY || cell->j == 0) ||
				(cell->k == WCellsNumZ || cell->k == 0))
			{
				color = vec3(0.0, 0.0, 0.0);
			}
			else
				color = vec3(0.27, 0.55, 0.65);

			// creates model matrix
			vec3 position = get_world_coordinates_from_world_cell_coordinates(
				cell->i, cell->j, cell->k
			);
			glm::mat4 model = translate(Mat4Identity, position);
			model = scale(model, vec3{WCellLenMeters, WCellLenMeters, WCellLenMeters});

			//render
			shader->Use();
			shader->SetFloat3("color", color);
			shader->SetFloat("opacity", 0.85);
			shader->SetMatrix4("model", model);
			shader->SetMatrix4("view", camera->mat_view);
			shader->SetMatrix4("projection", camera->mat_projection);
			glDisable(GL_CULL_FACE);
			render_mesh(cell_mesh, opts);
			glEnable(GL_CULL_FACE);
		}
	}


	inline void render_lightbulbs(Camera* camera, World* world)
	{
		auto mesh = GeometryCatalogue.find("lightbulb")->second;
		auto shader = ShaderCatalogue.find("color")->second;

		shader->SetMatrix4("view", camera->mat_view);
		shader->SetMatrix4("projection", camera->mat_projection);

		auto selected_light = EdContext.lights_panel.selected_light;
		auto selected_light_type = EdContext.lights_panel.selected_light_type;

		// point lights
		int point_c = 0;
		for(const auto& light : world->point_lights)
		{
			auto model = translate(Mat4Identity, light->position + vec3{0, 0.5, 0});
			model = scale(model, vec3{0.1f});
			RenderOptions opts;
			//opts.wireframe = true;
			//render
			shader->Use();
			shader->SetMatrix4("model", model);
			shader->SetFloat3("color", light->diffuse);
			shader->SetFloat("opacity", 1.0);

			render_mesh(mesh, opts);

			point_c++;
		}

		// spot lights
		int spot_c = 0;
		for(const auto& light : world->spot_lights)
		{
			auto model = translate(Mat4Identity, light->position + vec3{0, 0.5, 0});
			model = scale(model, vec3{0.1f});
			RenderOptions opts;
			//opts.wireframe = true;
			//render
			shader->Use();
			shader->SetMatrix4("model", model);
			shader->SetFloat3("color", light->diffuse);
			shader->SetFloat("opacity", 1.0);
			render_mesh(mesh, opts);
			spot_c++;
		}

		// render selection box and dir arrow for selected lightbulb
		if(selected_light >= 0)
		{
			vec3 light_position;
			vec3 light_direction;
			if(selected_light_type == "point")
			{
				assert(selected_light <= point_c);
				auto& light = *world->point_lights[selected_light];
				light_position = light.position;
			}
			else if(selected_light_type == "spot")
			{
				assert(selected_light <= spot_c);
				auto& light = *world->spot_lights[selected_light];
				light_position = light.position;
				light_direction = light.direction;
			}

			// selection box
			auto aabb_mesh = GeometryCatalogue.find("aabb")->second;

			auto aabb_model = translate(Mat4Identity, light_position - vec3{0.1575, 0, 0.1575});
			aabb_model = scale(aabb_model, vec3{0.3f, 0.6f, 0.3f});
			RenderOptions opts;
			opts.wireframe = true;

			shader->Use();
			shader->SetMatrix4("model", aabb_model);
			shader->SetFloat3("color", vec3{0.9, 0.7, 0.9});
			shader->SetFloat("opacity", 1.0);

			render_mesh(aabb_mesh, opts);

			// direction arrow
			if(selected_light_type == "spot")
			{
				float pitch, yaw;
				compute_angles_from_direction(pitch, yaw, light_direction);
				vec3 arrow_direction = compute_direction_from_angles(pitch, yaw);

				vec3 arrow_origin = light_position - vec3{0.0, 0.56, 0.0};
				vec3 arrow_end = arrow_origin + arrow_direction * 1.5f;
				ImDraw::AddLine(IMHASH, arrow_origin, arrow_end, 1.5);
			}

			// @todo: epic fail below (trying to rotate an arrow mesh according to a dir vector)
			// auto arrow_mesh = Geometry_Catalogue.find("axis")->second;
			// vec3 front = arrow_origin + light.direction;
			// vec3 up = glm::cross(arrow_origin, );

			// @todo: this is a workaround, since we are not using quaternions yet, we must
			//       be careful with 0/180 degree angles between up and direction vectors
			//       using glm::lookAt()
			// @todo: Actually now we are using immediate draw and lines.

			// //mat4 arrow_model = translate(Mat4Identity, arrow_origin);
			// mat4 arrow_model = 
			//    glm::translate(Mat4Identity, arrow_origin) *
			//    glm::rotate(Mat4Identity, glm::radians(90.0f), vec3(1, 0, 0)) *
			//    glm::lookAt(vec3{0.0}, arrow_direction, vec3{0,1,0})
			// ;
			// //arrow_model = glm::scale(arrow_model, vec3{0.2f, 0.3f, 0.2f});

			//render arrow
			// shader->use();
			// shader->setFloat3("color", vec3{0.9, 0.7, 0.9});
			// shader->setFloat("opacity", 1.0);
			// shader->setMatrix4("model", arrow_model);
			// shader->setMatrix4("view", camera->View4x4);
			// shader->setMatrix4("projection", camera->Projection4x4);
			// render_mesh(arrow_mesh, RenderOptions{});
		}
	}


	void render_entity_control_arrows(EntityPanelContext* panel, World* world, Camera* camera)
	{
		//@todo: try placing editor objects in a separate z buffer? Maybe manually... so we don't have to use GL_ALWAYS
		glDepthFunc(GL_ALWAYS);
		render_editor_entity(panel->x_arrow, world, camera);
		render_editor_entity(panel->y_arrow, world, camera);
		render_editor_entity(panel->z_arrow, world, camera);
		glDepthFunc(GL_LESS);
	}


	void render_entity_rotation_gizmo(EntityPanelContext* panel, World* world, Camera* camera)
	{
		//@todo: try placing editor objects in a separate z buffer? Maybe manually... so we don't have to use GL_ALWAYS
		glDepthFunc(GL_ALWAYS);
		render_editor_entity(panel->rotation_gizmo_x, world, camera);
		render_editor_entity(panel->rotation_gizmo_y, world, camera);
		render_editor_entity(panel->rotation_gizmo_z, world, camera);
		glDepthFunc(GL_LESS);
	}


	float _get_gizmo_scaling_factor(Entity* entity, float min, float max)
	{
		/* Editor gizmos need to follow entities' dimensions so they don't look too big or too small in comparison with the entity 
		   when displayed. */

		float scaling_factor = min;
		float min_dimension = MaxFloat;
		if(entity->scale.x < min_dimension)
			min_dimension = entity->scale.x;
		if(entity->scale.y < min_dimension)
			min_dimension = entity->scale.y;
		if(entity->scale.z < min_dimension)
			min_dimension = entity->scale.z;

		if(min_dimension < min)
			scaling_factor = min_dimension;
		else if(min_dimension >= max)
			scaling_factor = min_dimension / max;

		return scaling_factor;
	}


	void update_entity_control_arrows(EntityPanelContext* panel)
	{
		// arrow positioning settings
		float angles[3] = {270, 0, 90};
		Entity* arrows[3] = {panel->x_arrow, panel->y_arrow, panel->z_arrow};
		vec3 rot_axis[3] = {UnitZ, UnitX, UnitX};

		auto entity = panel->entity;

		if(panel->reverse_scale)
		{
			for(int i = 0; i < 3; i++)
				angles[i] += 180;
		}

		// update arrow mat models doing correct matrix multiplication order
		auto starting_model = translate(Mat4Identity, entity->position);
		starting_model = rotate(starting_model, glm::radians(entity->rotation.x), UnitX);
		starting_model = rotate(starting_model, glm::radians(entity->rotation.y), UnitY);
		starting_model = rotate(starting_model, glm::radians(entity->rotation.z), UnitZ);

		float scale_value = _get_gizmo_scaling_factor(entity, 0.8, 3.0);

		for(int i = 0; i < 3; i++)
		{
			auto arrow = arrows[i];
			auto model = rotate(starting_model, glm::radians(angles[i]), rot_axis[i]);
			model = scale(model, vec3(scale_value));
			arrow->mat_model = model;
			arrow->UpdateCollider();
			arrow->UpdateBoundingBox();
		}
	}


	void update_entity_rotation_gizmo(EntityPanelContext* panel)
	{
		// arrow positioning settings
		float angles[3] = {270, 0, 90};
		vec3 rot_axis[3] = {UnitZ, UnitX, UnitX};
		Entity* gizmos[3] = {panel->rotation_gizmo_x, panel->rotation_gizmo_y, panel->rotation_gizmo_z};

		auto entity = panel->entity;

		// update arrow mat models doing correct matrix multiplication order
		auto starting_model = translate(Mat4Identity, entity->bounding_box.GetCentroid());

		float scale_value = _get_gizmo_scaling_factor(entity, 1.0, 3.0);

		for(int i = 0; i < 3; i++)
		{
			auto model = rotate(starting_model, glm::radians(angles[i]), rot_axis[i]);
			model = scale(model, vec3(scale_value));
			gizmos[i]->mat_model = model;
			gizmos[i]->UpdateCollider();
			gizmos[i]->UpdateBoundingBox();
		}
	}


	void render_entity_mesh_normals(EntityPanelContext* panel)
	{
		// only for aabb
		auto entity = panel->entity;

		int triangles = entity->mesh->indices.size() / 3;
		for(int i = 0; i < triangles; i++)
		{
			Triangle _t = get_triangle_for_indexed_mesh(entity->mesh, entity->mat_model, i);
			vec3 normal = triangleNormal(_t.a, _t.b, _t.c);
			Face f = face_from_axis_aligned_triangle(_t);

			ImDraw::AddPoint(IMHASH, f.center, 2.0, true);

			ImDraw::AddLine(IMHASH, f.center, f.center + normal * 2.0f, 2.5, true);
		}
	}


	void check_selection_to_open_panel(Player* player, World* world, Camera* camera)
	{
		auto* GII = GlobalInputInfo::Get();
		auto pickray = cast_pickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		auto test = world->Raycast(pickray, RayCast_TestOnlyVisibleEntities);
		auto test_light = world->RaycastLights(pickray);

		if(test.hit && (!test_light.hit || test_light.distance > test.distance))
		{
			if(test.entity->name == PlayerName)
				open_player_panel(player);
			else
				open_entity_panel(test.entity);

		}
		else if(test_light.hit)
			open_lights_panel(test_light.obj_hit_type, test_light.obj_hit_index, true);
	}


	void check_selection_to_select_related_entity(World* world, Camera* camera)
	{
		auto* EM = EntityManager::Get();
		auto* GII = GlobalInputInfo::Get();

		auto pickray = cast_pickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		auto test = world->Raycast(pickray, RayCast_TestOnlyVisibleEntities);
		if(test.hit)
		{
			EdContext.select_entity_aux_mode = false;
			*EdContext.select_entity_aux_mode_entity_slot = test.entity;

			if(EdContext.select_entity_aux_mode_callback != EdToolCallback_NoCallback)
			{
				switch(EdContext.select_entity_aux_mode_callback)
				{
					case EdToolCallback_EntityManagerSetType:
					{
						EM->SetType(
							*EdContext.select_entity_aux_mode_entity_slot,
							EdContext.select_entity_aux_mode_callback_args.entity_type
						);
						break;
					}
				}
			}
		}
	}


	void check_selection_to_move_entity(World* world, Camera* camera)
	{
		auto* GII = GlobalInputInfo::Get();

		auto pickray = cast_pickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		auto test = world->Raycast(pickray, RayCast_TestOnlyVisibleEntities);
		auto test_light = world->RaycastLights(pickray);
		if(test.hit && (!test_light.hit || test_light.distance > test.distance))
			activate_move_mode(test.entity);
		else if(test_light.hit)
			activate_move_light_mode(test_light.obj_hit_type, test_light.obj_hit_index);
	}


	bool check_selection_to_grab_entity_arrows(Camera* camera)
	{
		auto* GII = GlobalInputInfo::Get();

		auto pickray = cast_pickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		RaycastTest test;

		Entity* arrows[3] = {EdContext.entity_panel.x_arrow, EdContext.entity_panel.y_arrow, EdContext.entity_panel.z_arrow};

		For(3)
		{
			test = test_ray_against_entity(pickray, arrows[i]);
			if(test.hit)
			{
				activate_move_entity_by_arrow(i + 1);
				return true;
			}
		}

		return false;
	}


	bool check_selection_to_grab_entity_rotation_gizmo(Camera* camera)
	{
		auto* GII = GlobalInputInfo::Get();

		auto pickray = cast_pickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		RaycastTest test;

		Entity* rot_gizmos[3] = {
		EdContext.entity_panel.rotation_gizmo_x,
		EdContext.entity_panel.rotation_gizmo_y,
		EdContext.entity_panel.rotation_gizmo_z
		};

		For(3)
		{
			test = test_ray_against_entity(pickray, rot_gizmos[i]);
			if(test.hit)
			{
				activate_rotate_entity_with_mouse(i + 1);
				return true;
			}
		}

		return false;
	}


	void start_dear_imgui_frame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}


	void end_dear_imgui_frame()
	{
		ImGui::EndFrame();
	}

}
