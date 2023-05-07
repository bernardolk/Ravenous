#include <glfw3.h>
#include <imgui.h>
#include "editor.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "tools/editor_tools.h"
#include <editor/editor_toolbar.h>
#include <editor/editor_entity_panel.h>
#include <editor/editor_player_panel.h>
#include <editor/editor_world_panel.h>
#include <editor/editor_palette_panel.h>
#include <editor/editor_lights_panel.h>
#include <editor/editor_collision_log_panel.h>
#include <editor/editor_input_recorder_panel.h>
#include <editor/editor_scene_objects_panel.h>
#include "editor/editor_colors.h"
#include "engine/utils/colors.h"
#include "game/entities/player.h"
#include "engine/render/renderer.h"
#include "engine/utils/utils.h"
#include "engine/camera/camera.h"
#include "engine/entities/lights.h"
#include "engine/io/loaders.h"
#include "engine/geometry/vertex.h"
#include "engine/io/display.h"
#include "engine/io/input.h"
#include "engine/render/im_render.h"
#include "engine/render/shader.h"
#include "engine/render/text/face.h"
#include "engine/render/text/text_renderer.h"
#include "engine/world/world.h"
#include "engine/collision/cl_edge_detection.h"

namespace Editor
{
	void StartDearImguiFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}


	void EndDearImguiFrame()
	{
		ImGui::EndFrame();
	}

	//------------------
	// > UPDATE EDITOR
	//------------------

	void Update(Player* player, T_World* world, Camera* camera)
	{
		auto& ed_context = *GetContext();
		
		string& scene_name = T_World::Get()->scene_name;

		if (ed_context.last_frame_scene != scene_name)
		{
			ed_context.entity_panel.active = false;
			ed_context.world_panel.active = false;
		}

		ed_context.last_frame_scene = scene_name;

		// check for asset changes
		// CheckForAssetChanges();
		UpdateTriaxisGizmo();

		// ENTITY PANEL
		if (!ed_context.entity_panel.active)
		{
			ed_context.entity_panel.rename_buffer[0] = 0;
			ed_context.snap_mode = false;
			ed_context.stretch_mode = false;
			ed_context.snap_reference = nullptr;
		}

		// unselect lights when not panel is not active
		if (!ed_context.lights_panel.active)
		{
			ed_context.lights_panel.selected_light = -1;
			ed_context.lights_panel.selected_light_type = "";
		}
		else if (
			ed_context.lights_panel.selected_light != -1 &&
			ed_context.lights_panel.selected_light_type != ""
		)
		{
			ed_context.show_lightbulbs = true;
		}


		// set editor mode values to initial if not active
		if (!ed_context.measure_mode)
		{
			ed_context.first_point_found = false;
			ed_context.second_point_found = false;
		}
		if (!ed_context.snap_mode)
		{
			ed_context.snap_cycle = 0;
			ed_context.snap_axis = 1;
			ed_context.snap_reference = nullptr;
		}

		// respond to mouse if necessary
		if (ed_context.move_mode)
		{
			if (ed_context.mouse_click)
			{
				if (ed_context.selected_light > -1)
					PlaceLight();
				else
					PlaceEntity(world);
			}
			else
			{
				if (ed_context.selected_light > -1)
					MoveLightWithMouse(ed_context.selected_light_type, ed_context.selected_light, world);
				else
					MoveEntityWithMouse(ed_context.selected_entity);
			}
		}

		if (ed_context.select_entity_aux_mode)
		{
			if (ed_context.mouse_click)
			{
				CheckSelectionToSelectRelatedEntity(world, camera);
			}
		}

		if (ed_context.move_entity_by_arrows)
		{
			if (ed_context.mouse_dragging)
				MoveEntityByArrows(ed_context.selected_entity);
				// the below condition is to prevent from deactivating too early
			else if (!ed_context.mouse_click)
				PlaceEntity(world);
		}

		if (ed_context.rotate_entity_with_mouse)
		{
			if (ed_context.mouse_dragging)
				RotateEntityWithMouse(ed_context.selected_entity);
				// the below condition is to prevent from deactivating too early
			else if (!ed_context.mouse_click)
				PlaceEntity(world);
		}


		if (ed_context.place_mode)
		{
			if (ed_context.mouse_click)
				PlaceEntity(world);
			else
				SelectEntityPlacingWithMouseMove(ed_context.selected_entity, world);
		}

		if (ed_context.scale_entity_with_mouse)
		{
			ScaleEntityWithMouse(ed_context.selected_entity);
		}

		// resets mouse click event
		ed_context.mouse_click = false;

		// check for debug flags
		if (ed_context.debug_ledge_detection)
		{
			CL_PerformLedgeDetection(player, world);
		}
	}

	void UpdateTriaxisGizmo()
	{
		auto& ed_context = *GetContext();

		for (int i = 0; i < 3; i++)
		{
			auto entity = ed_context.tri_axis[i];
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

	void Render(Player* player, T_World* world, Camera* camera)
	{
		auto& ed_context = *GetContext();

		// render world objs if toggled
		if (ed_context.show_event_triggers)
		{
			RenderEventTriggers(camera, world);
		}

		if (ed_context.show_world_cells)
		{
			RenderWorldCells(camera, world);
		}

		if (ed_context.show_lightbulbs)
		{
			RenderLightbulbs(camera, world);
		}

		// render triaxis
		auto triaxis_view = lookAt(vec3(0.0f), camera->front, -1.0f * camera->up);
		float displacement_x[3] = {0.3f, 0.0f, 0.0f};
		float displacement_y[3] = {0.0f, 0.3f, 0.0f};
		for (int i = 0; i < 3; i++)
		{
			// ref. axis
			auto axis = ed_context.tri_axis[i];
			axis->shader->Use();
			axis->shader->SetMatrix4("model", axis->mat_model);
			axis->shader->SetMatrix4("view", triaxis_view);
			axis->shader->SetFloat2("screenPos", TriaxisScreenposX, TriaxisScreenposY);
			RenderEntity(axis);
		}

		// Entity panel special render calls
		if (ed_context.entity_panel.active)
		{
			// Render glowing pink wireframe on top of selected entity
			{
				// update
				auto state = GetEntityState(ed_context.selected_entity);
				auto model = MatModelFromEntityState(state);

				// compute color intensity based on time
				float time_value = glfwGetTime();
				float intensity = sin(time_value) * 2;
				if (intensity < 0)
					intensity *= -1.0;
				intensity += 1.0;

				// render
				auto glowing_line = ShaderCatalogue.find("color")->second;
				glowing_line->Use();
				glowing_line->SetMatrix4("model", model);
				glowing_line->SetFloat3("color", intensity * 0.890, intensity * 0.168, intensity * 0.6);
				glowing_line->SetFloat("opacity", 1);
				RenderMesh(ed_context.selected_entity->mesh, RenderOptions{true, false, 3});
			}

			// Render glowing yellow wireframe on top of an arbitrary related entity
			if (ed_context.entity_panel.show_related_entity)
			{
				// update
				auto state = GetEntityState(ed_context.entity_panel.related_entity);
				auto model = MatModelFromEntityState(state);

				// compute color intensity based on time
				float time_value = glfwGetTime();
				float intensity = sin(time_value) * 2;
				if (intensity < 0)
					intensity *= -1.0;
				intensity += 1.0;

				// render
				auto glowing_line = ShaderCatalogue.find("color")->second;
				glowing_line->Use();
				glowing_line->SetMatrix4("model", model);
				glowing_line->SetFloat3("color", intensity * 0.941, intensity * 0.776, intensity * 0);
				glowing_line->SetFloat("opacity", 1);
				RenderMesh(ed_context.entity_panel.related_entity->mesh, RenderOptions{true, false, 3});
			}
		}

		// render glowing wireframe on top of snap reference entity
		if (ed_context.snap_mode && ed_context.snap_reference != nullptr)
		{
			// update
			auto state = GetEntityState(ed_context.snap_reference);
			auto model = MatModelFromEntityState(state);

			// compute color intensity based on time
			float time_value = glfwGetTime();
			float intensity = sin(time_value) * 2;
			if (intensity < 0)
				intensity *= -1.0;
			intensity += 1.0;

			// render
			auto glowing_line = ShaderCatalogue.find("color")->second;
			glowing_line->Use();
			glowing_line->SetMatrix4("model", model);
			glowing_line->SetFloat3("color", intensity * 0.952, intensity * 0.843, intensity * 0.105);
			glowing_line->SetFloat("opacity", 1);
			RenderMesh(ed_context.snap_reference->mesh, RenderOptions{true, false, 3});
		}

		// --------------
		// render panels
		// --------------
		if (ed_context.scene_objects_panel.active)
			RenderSceneObjectsPanel(world, &ed_context.scene_objects_panel);

		if (ed_context.world_panel.active)
			RenderWorldPanel(&ed_context.world_panel, world, player);

		if (ed_context.entity_panel.active)
		{
			auto& panel = ed_context.entity_panel;

			RenderEntityPanel(&panel, world);
			RenderEntityControlArrows(&panel, world, camera);
			RenderEntityRotationGizmo(&panel, world, camera);

			if (panel.show_normals)
				RenderEntityMeshNormals(&panel);
			// @TODO: Some bug being caused in this call
			//if(panel.show_collider)
			//   ImDraw::add_mesh(IMHASH, &panel.entity->collider, COLOR_PURPLE_1, 0);
			if (panel.show_bounding_box)
			{
				auto aabb = GeometryCatalogue.find("aabb")->second;
				auto [pos, scale] = panel.entity->bounding_box.GetPosAndScale();
				ImDraw::AddMesh(IMHASH, aabb, pos, vec3(0), scale, COLOR_PINK_1, 0);
			}
		}

		if (ed_context.player_panel.active)
		{
			RenderPlayerPanel(&ed_context.player_panel);
		}

		if (ed_context.palette_panel.active)
			RenderPalettePanel(&ed_context.palette_panel);

		if (ed_context.lights_panel.active)
			RenderLightsPanel(&ed_context.lights_panel, world);

		if (ed_context.input_recorder_panel.active)
			RenderInputRecorderPanel(&ed_context.input_recorder_panel);

		if (ed_context.collision_log_panel.active)
			RenderCollisionLogPanel(&ed_context.collision_log_panel);

		// -----------------------
		// render gizmos inscreen
		// -----------------------
		if (ed_context.measure_mode && ed_context.first_point_found && ed_context.second_point_found)
		{
			auto render_opts = RenderOptions();
			render_opts.always_on_top = true;
			render_opts.line_width = 2.0;
			render_opts.color = EdRed;

			vec3 second_point;
			if (ed_context.measure_axis == 0)
				second_point = vec3(ed_context.measure_to, ed_context.measure_from.y, ed_context.measure_from.z);
			if (ed_context.measure_axis == 1)
				second_point = vec3(ed_context.measure_from.x, ed_context.measure_to, ed_context.measure_from.z);
			if (ed_context.measure_axis == 2)
				second_point = vec3(ed_context.measure_from.x, ed_context.measure_from.y, ed_context.measure_to);

			ImDraw::Add(
				IMHASH,
				std::vector<Vertex>{
				Vertex{ed_context.measure_from},
				Vertex{second_point}
				},
				GL_LINE_LOOP,
				render_opts
			);
		}

		if (ed_context.locate_coords_mode && ed_context.locate_coords_found_point)
		{
			ImDraw::AddPoint(IMHASH, ed_context.locate_coords_position, 2.0);
		}

		RenderToolbar(world);

		RenderTextOverlay(player, camera);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Terminate()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}


	void Initialize()
	{
		auto& ed_context = *GetContext();

		ImGui::CreateContext();
		auto& io = ImGui::GetIO();
		ImGui_ImplGlfw_InitForOpenGL(GlobalDisplayConfig::GetWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 330");

		ImGui::StyleColorsDark();
		ed_context.im_style = &ImGui::GetStyle();
		ed_context.im_style->WindowRounding = 1.0f;

		// load tri axis gizmo
		const auto axis_mesh = LoadWavefrontObjAsMesh(Paths::Models, "axis");

		auto x_axis = new E_Entity();
		auto y_axis = new E_Entity();
		auto z_axis = new E_Entity();

		x_axis->mesh = axis_mesh;
		y_axis->mesh = axis_mesh;
		z_axis->mesh = axis_mesh;

		const auto blue_tex = LoadTextureFromFile("blue.jpg", Paths::Textures);
		const auto green_tex = LoadTextureFromFile("green.jpg", Paths::Textures);
		const auto red_tex = LoadTextureFromFile("red.jpg", Paths::Textures);

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

		ed_context.tri_axis[0] = x_axis;
		ed_context.tri_axis[1] = y_axis;
		ed_context.tri_axis[2] = z_axis;


		// load entity panel axis arrows
		// @todo: refactor this to use the entity_manager
		auto x_arrow = new E_Entity();
		auto y_arrow = new E_Entity();
		auto z_arrow = new E_Entity();

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

		ed_context.entity_panel.x_arrow = x_arrow;
		ed_context.entity_panel.y_arrow = y_arrow;
		ed_context.entity_panel.z_arrow = z_arrow;

		// creates entity rotation gizmos
		ed_context.entity_panel.rotation_gizmo_x = new E_Entity();
		SetEntityAssets(ed_context.entity_panel.rotation_gizmo_x, {
		.name = "rotation_gizmo_x",
		.mesh = "rotation_gizmo",
		.shader = "ed_entity_arrow_shader",
		.texture = "red",
		.collision_mesh = "rotation_gizmo_collision"});

		ed_context.entity_panel.rotation_gizmo_y = new E_Entity();
		SetEntityAssets(ed_context.entity_panel.rotation_gizmo_y,{
		.name = "rotation_gizmo_y",
		.mesh = "rotation_gizmo",
		.shader = "ed_entity_arrow_shader",
		.texture = "green",
		.collision_mesh = "rotation_gizmo_collision"});

		ed_context.entity_panel.rotation_gizmo_z = new E_Entity();
		SetEntityAssets(ed_context.entity_panel.rotation_gizmo_z, {
		.name = "rotation_gizmo_z",
		.mesh = "rotation_gizmo",
		.shader = "ed_entity_arrow_shader",
		.texture = "blue",
		.collision_mesh = "rotation_gizmo_collision"});


		// palette panel
		InitializePalette(&ed_context.palette_panel);

		ed_context.last_frame_scene = T_World::Get()->scene_name;
	}


	void RenderTextOverlay(Player* player, Camera* camera)
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
		RenderText(font, 235, 45, camera_position);


		// PLAYER POSITION
		vec3 p_feet = player->GetFeetPosition();
		std::string player_p[3]{
		FormatFloatTostr(p_feet.x, 1),
		FormatFloatTostr(p_feet.y, 1),
		FormatFloatTostr(p_feet.z, 1),
		};
		std::string player_pos = "player:   x: " + player_p[0] + " y: " + player_p[1] + " z: " + player_p[2];
		RenderText(font, 235, 70, player_pos);


		// PLAYER LIVES
		std::string lives = std::to_string(player->lives);
		RenderText(
			font,
			GlobalDisplayConfig::viewport_width - 400,
			90,
			player->lives == 2 ? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1},
			lives
		);


		// PLAYER STATE
		auto player_state_text_color = vec3(0, 0, 0);
		std::string player_state_text;
		switch (player->player_state)
		{
			case PlayerState::Standing:
				player_state_text = "PLAYER PlayerState::Standing";
				break;
			case PlayerState::Falling:
				player_state_text = "PLAYER FALLING";
				break;
			case PlayerState::Jumping:
				player_state_text = "PLAYER JUMPING";
				break;
			case PlayerState::Sliding:
				player_state_text = "PLAYER SLIDING";
				break;
			case PlayerState::SlideFalling:
				player_state_text = "PLAYER SLIDE FALLING";
				break;
		}
		RenderText("consola18", GlobalDisplayConfig::viewport_width - 400, 30, player_state_text_color, player_state_text);


		std::string player_floor = "player floor: ";
		if (player->standing_entity_ptr != nullptr)
			player_floor += player->standing_entity_ptr->name;
		RenderText(GlobalDisplayConfig::viewport_width - 400, 60, player_floor);

		std::string p_grab = "grabbing: ";
		if (player->grabbing_entity != nullptr)
			p_grab += player->grabbing_entity->name;
		RenderText(GlobalDisplayConfig::viewport_width - 400, 45, p_grab);


		// FPS
		std::string fps = std::to_string(Rvn::frame.fps);
		std::string fps_gui = "FPS: " + fps;
		RenderText(font, GlobalDisplayConfig::viewport_width - 110, 40, fps_gui);


		// EDITOR TOOLS INDICATORS

		// ----------
		// SNAP MODE
		// ----------
		auto& ed_context = *GetContext();

		if (ed_context.snap_mode)
		{
			std::string snap_cycle;
			switch (ed_context.snap_cycle)
			{
				case 0:
					snap_cycle = "top";
					break;
				case 1:
					snap_cycle = "mid";
					break;
				case 2:
					snap_cycle = "bottom";
					break;
			}

			std::string snap_axis;
			switch (ed_context.snap_axis)
			{
				case 0:
					snap_axis = "X";
					break;
				case 1:
					snap_axis = "Y";
					break;
				case 2:
					snap_axis = "Z";
					break;
			}

			// if position is changed and not commited, render text yellow
			vec3 snap_mode_subtext_color;
			if (ed_context.snap_reference == nullptr)
				snap_mode_subtext_color = tool_text_color_yellow;
			else
			{
				auto state = ed_context.undo_stack.Check();
				if (state.entity != nullptr && state.position != ed_context.entity_panel.entity->position)
					snap_mode_subtext_color = tool_text_color_yellow;
				else
					snap_mode_subtext_color = tool_text_color_green;
			}

			// selects text based on situation of snap tool
			std::string sub_text;
			if (ed_context.snap_reference == nullptr)
				sub_text = "select another entity to snap to.";
			else
				sub_text = "press Enter to commit position. x/y/z to change axis.";

			RenderText(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				tool_text_color_yellow,
				true,
				"SNAP MODE (" + snap_axis + "-" + snap_cycle + ")"
			);

			RenderText(
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
		if (ed_context.measure_mode)
		{
			std::string axis =
			ed_context.measure_axis == 0 ? "x" :
			ed_context.measure_axis == 1 ? "y" :
			"z";

			RenderText(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"MEASURE MODE (" + axis + ")"
			);

			if (ed_context.second_point_found)
			{
				float dist_ref =
				ed_context.measure_axis == 0 ? ed_context.measure_from.x :
				ed_context.measure_axis == 1 ? ed_context.measure_from.y :
				ed_context.measure_from.z;

				RenderText(
					font_center,
					GlobalDisplayConfig::viewport_width / 2,
					centered_text_height_small,
					vec3(0.8, 0.8, 0.2),
					true,
					"(" + FormatFloatTostr(abs(ed_context.measure_to - dist_ref), 2) + " m)"
				);
			}
		}

		// ----------
		// MOVE MODE
		// ----------
		if (ed_context.move_mode)
		{
			std::string move_axis;
			switch (ed_context.move_axis)
			{
				case 0:
					move_axis = "XZ";
					break;
				case 1:
					move_axis = "X";
					break;
				case 2:
					move_axis = "Y";
					break;
				case 3:
					move_axis = "Z";
					break;
			}

			RenderText(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"MOVE MODE (" + move_axis + ")"
			);

			RenderText(
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
		if (ed_context.place_mode)
		{
			RenderText(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"PLACE MODE"
			);

			RenderText(
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
		if (ed_context.locate_coords_mode)
		{
			RenderText(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"LOCATE COORDS MODE"
			);

			std::string locate_coords_subtext;
			if (!ed_context.locate_coords_found_point)
			{
				locate_coords_subtext = "Please select a world position to get coordinates.";
			}
			else
			{
				locate_coords_subtext =
				"(x: " + FormatFloatTostr(ed_context.locate_coords_position[0], 2) +
				", y: " + FormatFloatTostr(ed_context.locate_coords_position[1], 2) +
				", z: " + FormatFloatTostr(ed_context.locate_coords_position[2], 2) + ")";
			}

			RenderText(
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
		if (ed_context.stretch_mode)
		{
			RenderText(
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
		if (ed_context.select_entity_aux_mode)
		{
			RenderText(
				font_center,
				GlobalDisplayConfig::viewport_width / 2,
				centered_text_height,
				vec3(0.8, 0.8, 0.2),
				true,
				"SELECT RELATED ENTITY"
			);
		}
	}

	//TODO: Reimplement
	void RenderEventTriggers(Camera* camera, T_World* world)
	{
		/*
		 if (world->interactables.size() == 0)
			 return;

		auto find = ShaderCatalogue.find("color");
		auto shader = find->second;

		shader->Use();
		shader->SetMatrix4("view", camera->mat_view);
		shader->SetMatrix4("projection", camera->mat_projection);

		for (int i = 0; i < world->interactables.size(); i++)
		{
			auto checkpoint = world->interactables[i];
			shader->SetMatrix4("model", checkpoint->trigger_mat_model);
			shader->SetFloat3("color", 0.5, 0.5, 0.3);
			shader->SetFloat("opacity", 0.6);
			RenderMesh(checkpoint->trigger, RenderOptions{});
		}
		*/
	}


	void RenderWorldCells(Camera* camera, T_World* world)
	{
		auto shader = ShaderCatalogue.find("color")->second;
		auto cell_mesh = GeometryCatalogue.find("aabb")->second;
		auto& ed_context = *GetContext();

		auto chunk_iterator = world->GetChunkIterator();
		while (auto* chunk = chunk_iterator())
		{
			RenderOptions opts;
			opts.wireframe = true;

			vec3 color;
			if (ed_context.world_panel.chunk_position_vec.x == chunk->i &&
				ed_context.world_panel.chunk_position_vec.y == chunk->j &&
				ed_context.world_panel.chunk_position_vec.z == chunk->k)
			{
				opts.line_width = 1.5;
				color = vec3(0.8, 0.4, 0.2);
			}
			else if ((chunk->i == WorldChunkNumX || chunk->i == 0) ||
				(chunk->j == WorldChunkNumY || chunk->j == 0) ||
				(chunk->k == WorldChunkNumZ || chunk->k == 0))
			{
				color = vec3(0.0, 0.0, 0.0);
			}
			else
				color = vec3(0.27, 0.55, 0.65);

			// creates model matrix
			vec3 position = GetWorldCoordinatesFromWorldCellCoordinates(
				chunk->i, chunk->j, chunk->k
			);
			glm::mat4 model = translate(Mat4Identity, position);
			model = scale(model, vec3{WorldChunkLengthMeters, WorldChunkLengthMeters, WorldChunkLengthMeters});

			//render
			shader->Use();
			shader->SetFloat3("color", color);
			shader->SetFloat("opacity", 0.85);
			shader->SetMatrix4("model", model);
			shader->SetMatrix4("view", camera->mat_view);
			shader->SetMatrix4("projection", camera->mat_projection);
			glDisable(GL_CULL_FACE);
			RenderMesh(cell_mesh, opts);
			glEnable(GL_CULL_FACE);
		}
	}


	inline void RenderLightbulbs(Camera* camera, T_World* world)
	{
		auto& ed_context = *GetContext();

		auto mesh = GeometryCatalogue.find("lightbulb")->second;
		auto shader = ShaderCatalogue.find("color")->second;

		shader->SetMatrix4("view", camera->mat_view);
		shader->SetMatrix4("projection", camera->mat_projection);

		auto selected_light = ed_context.lights_panel.selected_light;
		auto selected_light_type = ed_context.lights_panel.selected_light_type;

		// point lights
		int point_c = 0;
		for (const auto& light : world->point_lights)
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

			RenderMesh(mesh, opts);

			point_c++;
		}

		// spot lights
		int spot_c = 0;
		for (const auto& light : world->spot_lights)
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
			RenderMesh(mesh, opts);
			spot_c++;
		}

		// render selection box and dir arrow for selected lightbulb
		if (selected_light >= 0)
		{
			vec3 light_position;
			vec3 light_direction;
			if (selected_light_type == "point")
			{
				assert(selected_light <= point_c);
				auto& light = *world->point_lights[selected_light];
				light_position = light.position;
			}
			else if (selected_light_type == "spot")
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

			RenderMesh(aabb_mesh, opts);

			// direction arrow
			if (selected_light_type == "spot")
			{
				float pitch, yaw;
				CameraManager::ComputeAnglesFromDirection(pitch, yaw, light_direction);
				vec3 arrow_direction = ComputeDirectionFromAngles(pitch, yaw);

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


	void RenderEntityControlArrows(EntityPanelContext* panel, T_World* world, Camera* camera)
	{
		//@todo: try placing editor objects in a separate z buffer? Maybe manually... so we don't have to use GL_ALWAYS
		glDepthFunc(GL_ALWAYS);
		RenderEditorEntity(panel->x_arrow, world, camera);
		RenderEditorEntity(panel->y_arrow, world, camera);
		RenderEditorEntity(panel->z_arrow, world, camera);
		glDepthFunc(GL_LESS);
	}


	void RenderEntityRotationGizmo(EntityPanelContext* panel, T_World* world, Camera* camera)
	{
		//@todo: try placing editor objects in a separate z buffer? Maybe manually... so we don't have to use GL_ALWAYS
		glDepthFunc(GL_ALWAYS);
		RenderEditorEntity(panel->rotation_gizmo_x, world, camera);
		RenderEditorEntity(panel->rotation_gizmo_y, world, camera);
		RenderEditorEntity(panel->rotation_gizmo_z, world, camera);
		glDepthFunc(GL_LESS);
	}


	float GetGizmoScalingFactor(E_Entity* entity, float min, float max)
	{
		/* Editor gizmos need to follow entities' dimensions so they don't look too big or too small in comparison with the entity 
		   when displayed. */

		float scaling_factor = min;
		float min_dimension = MaxFloat;
		if (entity->scale.x < min_dimension)
			min_dimension = entity->scale.x;
		if (entity->scale.y < min_dimension)
			min_dimension = entity->scale.y;
		if (entity->scale.z < min_dimension)
			min_dimension = entity->scale.z;

		if (min_dimension < min)
			scaling_factor = min_dimension;
		else if (min_dimension >= max)
			scaling_factor = min_dimension / max;

		return scaling_factor;
	}


	void UpdateEntityControlArrows(EntityPanelContext* panel)
	{
		// arrow positioning settings
		float angles[3] = {270, 0, 90};
		E_Entity* arrows[3] = {panel->x_arrow, panel->y_arrow, panel->z_arrow};
		vec3 rot_axis[3] = {UnitZ, UnitX, UnitX};

		auto entity = panel->entity;

		if (panel->reverse_scale)
		{
			for (int i = 0; i < 3; i++)
				angles[i] += 180;
		}

		// update arrow mat models doing correct matrix multiplication order
		auto starting_model = translate(Mat4Identity, entity->position);
		starting_model = rotate(starting_model, glm::radians(entity->rotation.x), UnitX);
		starting_model = rotate(starting_model, glm::radians(entity->rotation.y), UnitY);
		starting_model = rotate(starting_model, glm::radians(entity->rotation.z), UnitZ);

		float scale_value = GetGizmoScalingFactor(entity, 0.8, 3.0);

		for (int i = 0; i < 3; i++)
		{
			auto arrow = arrows[i];
			auto model = rotate(starting_model, glm::radians(angles[i]), rot_axis[i]);
			model = scale(model, vec3(scale_value));
			arrow->mat_model = model;
			arrow->UpdateCollider();
			arrow->UpdateBoundingBox();
		}
	}


	void UpdateEntityRotationGizmo(EntityPanelContext* panel)
	{
		// arrow positioning settings
		float angles[3] = {270, 0, 90};
		vec3 rot_axis[3] = {UnitZ, UnitX, UnitX};
		E_Entity* gizmos[3] = {panel->rotation_gizmo_x, panel->rotation_gizmo_y, panel->rotation_gizmo_z};

		auto entity = panel->entity;

		// update arrow mat models doing correct matrix multiplication order
		auto starting_model = translate(Mat4Identity, entity->bounding_box.GetCentroid());

		float scale_value = GetGizmoScalingFactor(entity, 1.0, 3.0);

		for (int i = 0; i < 3; i++)
		{
			auto model = rotate(starting_model, glm::radians(angles[i]), rot_axis[i]);
			model = scale(model, vec3(scale_value));
			gizmos[i]->mat_model = model;
			gizmos[i]->UpdateCollider();
			gizmos[i]->UpdateBoundingBox();
		}
	}


	void RenderEntityMeshNormals(EntityPanelContext* panel)
	{
		// only for aabb
		auto entity = panel->entity;

		int triangles = entity->mesh->indices.size() / 3;
		for (int i = 0; i < triangles; i++)
		{
			Triangle _t = get_triangle_for_indexed_mesh(entity->mesh, entity->mat_model, i);
			vec3 normal = triangleNormal(_t.a, _t.b, _t.c);
			Face f = FaceFromAxisAlignedTriangle(_t);

			ImDraw::AddPoint(IMHASH, f.center, 2.0, true);

			ImDraw::AddLine(IMHASH, f.center, f.center + normal * 2.0f, 2.5, true);
		}
	}

	void CheckSelectionToOpenPanel(Player* player, T_World* world, Camera* camera)
	{
		auto* GII = GlobalInputInfo::Get();
		auto pickray = CastPickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		auto test = world->Raycast(pickray, RayCast_TestOnlyVisibleEntities);
		auto test_light = world->RaycastLights(pickray);

		if (test.hit && (!test_light.hit || test_light.distance > test.distance))
		{
			OpenEntityPanel(test.entity);
		}
		else if (test_light.hit)
			OpenLightsPanel(test_light.obj_hit_type, test_light.obj_hit_index, true);
	}


	void CheckSelectionToSelectRelatedEntity(T_World* world, Camera* camera)
	{
		auto* GII = GlobalInputInfo::Get();
		auto& ed_context = *GetContext();

		auto pickray = CastPickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		auto test = world->Raycast(pickray, RayCast_TestOnlyVisibleEntities);
		if (test.hit)
		{
			ed_context.select_entity_aux_mode = false;
			*ed_context.select_entity_aux_mode_entity_slot = test.entity;

			if (ed_context.select_entity_aux_mode_callback != EdToolCallback_NoCallback)
			{
				switch (ed_context.select_entity_aux_mode_callback)
				{
					case EdToolCallback_EntityManagerSetType:
					{
						// EM->SetType(
						// 	*ed_context.select_entity_aux_mode_entity_slot,
						// 	ed_context.select_entity_aux_mode_callback_args.entity_type
						// );
						break;
					}
				}
			}
		}
	}


	void CheckSelectionToMoveEntity(T_World* world, Camera* camera)
	{
		auto* GII = GlobalInputInfo::Get();

		auto pickray = CastPickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		auto test = world->Raycast(pickray, RayCast_TestOnlyVisibleEntities);
		auto test_light = world->RaycastLights(pickray);
		if (test.hit && (!test_light.hit || test_light.distance > test.distance))
			ActivateMoveMode(test.entity);
		else if (test_light.hit)
			ActivateMoveLightMode(test_light.obj_hit_type, test_light.obj_hit_index);
	}


	bool CheckSelectionToGrabEntityArrows(Camera* camera)
	{
		auto* GII = GlobalInputInfo::Get();
		auto& ed_context = *GetContext();

		auto pickray = CastPickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		RaycastTest test;

		E_Entity* arrows[3] = {ed_context.entity_panel.x_arrow, ed_context.entity_panel.y_arrow, ed_context.entity_panel.z_arrow};

		For(3)
		{
			test = CL_TestAgainstRay(pickray, arrows[i]);
			if (test.hit)
			{
				ActivateMoveEntityByArrow(i + 1);
				return true;
			}
		}

		return false;
	}


	bool CheckSelectionToGrabEntityRotationGizmo(Camera* camera)
	{
		auto* GII = GlobalInputInfo::Get();
		auto& ed_context = *GetContext();

		auto pickray = CastPickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
		RaycastTest test;

		E_Entity* rot_gizmos[3] = {
		ed_context.entity_panel.rotation_gizmo_x,
		ed_context.entity_panel.rotation_gizmo_y,
		ed_context.entity_panel.rotation_gizmo_z
		};

		For(3)
		{
			test = CL_TestAgainstRay(pickray, rot_gizmos[i]);
			if (test.hit)
			{
				ActivateRotateEntityWithMouse(i + 1);
				return true;
			}
		}

		return false;
	}
}
