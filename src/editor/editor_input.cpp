#include "editor_input.h"

#include <glfw3.h>

#include "editor.h"
#include <imgui.h>

#include "console/console.h"
#include "editor/editor_state.h"
#include "tools/editor_tools.h"
#include "game/entities/player.h"
#include "engine/camera/camera.h"
#include "engine/rvn.h"
#include "engine/collision/raycast.h"
#include "engine/io/display.h"
#include "engine/io/input_phase.h"
#include "engine/serialization/sr_config.h"
#include "engine/serialization/sr_world.h"
#include "engine/io/input.h"
#include "engine/world/world.h"

namespace Editor
{
	void HandleInputFlagsForEditorMode(InputFlags flags, World* world)
	{
		// ------------------------
		// EDITOR EDITING COMMANDS
		// ------------------------
		// commands that return once detected,
		// not allowing for more than one at a time
		// to be issued.
		auto& context = *Editor::GetContext();
		auto* player = Player::Get();
		auto& program_config = *ProgramConfig::Get();
		auto* manager = CameraManager::Get();
		auto* camera = manager->GetCurrentCamera();

		if (Pressed(flags, KEY_LEFT_CTRL) && PressedOnce(flags, KEY_Z))
		{
			// snap mode controls the undo stack while it is active.
			if (!context.snap_mode)
				context.undo_stack.Undo();
			return;
		}

		if (Pressed(flags, KEY_LEFT_CTRL) && PressedOnce(flags, KEY_S))
		{
			// save scene
			player->checkpoint_pos = player->position;
			WorldSerializer::SaveToFile();
			// set scene
			program_config.initial_scene = World::Get()->scene_name;
			ConfigSerializer::Save(program_config);
			Rvn::rm_buffer->Add("world state saved", 1200);
			return;
		}

		if (Pressed(flags, KEY_LEFT_CTRL) && PressedOnce(flags, KEY_Y))
		{
			// snap mode controls the undo stack while it is active.
			if (!context.snap_mode)
				context.undo_stack.Redo();
			return;
		}

		if (PressedOnce(flags, KEY_ESC))
		{
			if (Editor::CheckModesAreActive())
				DeactivateEditorModes();
			else if (context.entity_panel.active)
				context.entity_panel.active = false;
			else if (context.world_panel.active)
				context.world_panel.active = false;
			else if (context.lights_panel.active)
				context.lights_panel.active = false;
			return;
		}

		// TODO: abstract call to not have to import imgui.h
		if (ImGui::GetIO().WantCaptureKeyboard)
			return;

		//Disabled
		/*
		if (PressedOnce(flags, KEY_DELETE))
		{
			if (context.entity_panel.active && context.entity_panel.focused)
			{
				context.entity_panel.active = false;
				EditorEraseEntity(context.entity_panel.entity);
				return;
			}
			if (context.lights_panel.active && context.lights_panel.focused)
			{
				if (context.lights_panel.selected_light > -1)
				{
					EditorEraseLight(context.lights_panel.selected_light, context.lights_panel.selected_light_type, world);
				}
				return;
			}
		}
		*/
		

		// ------------------------------------
		// TOOLS / CAMERA / CHARACTER CONTROLS
		// ------------------------------------

		// --------------------
		// SNAP MODE SHORTCUTS
		// --------------------
		if (context.snap_mode == true)
		{
			if (PressedOnce(flags, KEY_ENTER))
			{
				SnapCommit();
			}
			if (PressedOnly(flags, KEY_X))
			{
				if (context.snap_axis == 0)
					context.snap_cycle = (context.snap_cycle + 1) % 3;
				else
				{
					ApplyState(context.snap_tracked_state);
					context.snap_cycle = 0;
					context.snap_axis = 0;
				}
				if (context.snap_reference != nullptr)
					SnapEntityToReference(context.entity_panel.entity);
			}
			if (PressedOnly(flags, KEY_Y))
			{
				if (context.snap_axis == 1)
					context.snap_cycle = (context.snap_cycle + 1) % 3;
				else
				{
					ApplyState(context.snap_tracked_state);
					context.snap_cycle = 0;
					context.snap_axis = 1;
				}
				if (context.snap_reference != nullptr)
					SnapEntityToReference(context.entity_panel.entity);
			}
			if (PressedOnly(flags, KEY_Z))
			{
				if (context.snap_axis == 2)
					context.snap_cycle = (context.snap_cycle + 1) % 3;
				else
				{
					ApplyState(context.snap_tracked_state);
					context.snap_cycle = 0;
					context.snap_axis = 2;
				}
				if (context.snap_reference != nullptr)
					SnapEntityToReference(context.entity_panel.entity);
			}
			if (PressedOnly(flags, KEY_I))
			{
				context.snap_inside = !context.snap_inside;
				if (context.snap_reference != nullptr)
					SnapEntityToReference(context.entity_panel.entity);
			}
		}

		// --------------------
		// MOVE MODE SHORTCUTS
		// --------------------
		if (context.move_mode == true)
		{
			if (Pressed(flags, KEY_X) && Pressed(flags, KEY_Z))
			{
				context.move_axis = 0;
			}
			if (PressedOnly(flags, KEY_X))
			{
				context.move_axis = 1;
			}
			if (PressedOnly(flags, KEY_Y))
			{
				context.move_axis = 2;
			}
			if (PressedOnly(flags, KEY_Z))
			{
				context.move_axis = 3;
			}
			if (PressedOnly(flags, KEY_M))
			{
				context.move_mode = false;
				context.place_mode = true;
				return;
			}
		}

		// ---------------------
		// PLACE MODE SHORTCUTS
		// ---------------------
		if (context.place_mode == true)
		{
			if (PressedOnly(flags, KEY_M))
			{
				context.place_mode = false;
				context.move_mode = true;
				return;
			}
		}

		// -------------------
		// CAMERA TYPE TOGGLE
		// -------------------
		if (PressedOnce(flags, KEY_T))
		{
			// toggle camera type
			if (camera->type == FREE_ROAM)
				manager->SetCameraToThirdPerson();
			else if (camera->type == THIRD_PERSON)
				manager->SetCameraToFreeRoam();
		}

		// ---------------
		// CLICK CONTROLS
		// ---------------

		// TODO: Refactor this whole thing: This checks for a CLICK then for modes to decide what todo.
		// I think this is not the best way to handle this, we should first check for MODE then for ACTION.
		// We can set things in context based on input flags, OR use flags directly.
		// Either way, there is code for handling clicks and etc both here and at editor_main which is confusing
		// and is, currently, causing some bugs in the editor.

		context.mouse_click = false;
		context.mouse_dragging = false;

		auto* GII = GlobalInputInfo::Get();

		if (GII->mouse_state & MOUSE_LB_CLICK)
		{
			if (context.snap_mode)
			{
				CheckSelectionToSnap();
			}
			else if (context.measure_mode)
			{
				CheckSelectionToMeasure(world);
			}
			else if (context.locate_coords_mode)
			{
				CheckSelectionToLocateCoords(world);
			}
			else if (context.stretch_mode)
			{
				CheckSelectionToStretch();
			}
			else if (flags.key_press & KEY_G)
			{
				CheckSelectionToMoveEntity(world, camera);
			}
			else
			{
				context.mouse_click = true;

				if (context.entity_panel.active)
				{
					if (context.select_entity_aux_mode)
						return;
					if (CheckSelectionToGrabEntityArrows(camera))
						return;
					if (CheckSelectionToGrabEntityRotationGizmo(camera))
						return;
				}

				if (context.move_mode || context.place_mode)
					return;

				CheckSelectionToOpenPanel(player, world, camera);
			}
		}

		else if (GII->mouse_state & MOUSE_LB_DRAGGING)
		{
			context.mouse_dragging = true;
		}
		else if (GII->mouse_state & MOUSE_LB_HOLD)
		{
			context.mouse_dragging = true;
		}


		// -------------------------------
		// SPAWN PLAYER ON MOUSE POSITION
		// -------------------------------
		if (PressedOnce(flags, KEY_C))
		{
			auto pickray = CastPickray(camera, GII->mouse_coords.x, GII->mouse_coords.y);
			auto test = world->Raycast(pickray);
			if (test.hit)
			{
				auto surface_point = CL_GetPointFromDetection(pickray, test);
				player->position = surface_point;
				player->player_state = PlayerState::Standing;
				player->velocity = vec3(0, 0, 0);
				player->Update();
			}
		}

		// --------------------------------------------
		// CONTROL KEY USAGE BLOCKED FROM HERE ONWARDS
		// --------------------------------------------
		if (Pressed(flags, KEY_LEFT_CTRL)) // this doesn't solve the full problem.
			return;

		// -------------------------
		// CAMERA MOVEMENT CONTROLS
		// -------------------------
		// @TODO: this sucks
		auto* editor_camera = manager->GetEditorCamera();
		float camera_speed = camera->type == THIRD_PERSON ? length(player->velocity) * Rvn::frame.duration : Rvn::frame.real_duration * editor_camera->acceleration;

		if (flags.key_press & KEY_LEFT_SHIFT)
		{
			camera_speed = camera_speed * 2;
		}

		if (flags.key_press & KEY_W)
		{
			camera->position += camera_speed * camera->front;
		}
		if (flags.key_press & KEY_A)
		{
			// @TODO: this sucks too
			if (camera->type == FREE_ROAM)
				camera->position -= camera_speed * normalize(glm::cross(camera->front, camera->up));
			else if (camera->type == THIRD_PERSON)
				camera->orbital_angle -= 0.025;
		}
		if (Pressed(flags, KEY_S))
		{
			camera->position -= camera_speed * camera->front;
		}
		if (flags.key_press & KEY_D)
		{
			if (camera->type == FREE_ROAM)
				camera->position += camera_speed * normalize(glm::cross(camera->front, camera->up));
			else if (camera->type == THIRD_PERSON)
				camera->orbital_angle += 0.025;
		}
		if (flags.key_press & KEY_Q)
		{
			camera->position -= camera_speed * camera->up;
		}
		if (flags.key_press & KEY_E)
		{
			camera->position += camera_speed * camera->up;
		}
		if (flags.key_press & KEY_O)
		{
			manager->CameraLookAt(camera, vec3(0.0f, 0.0f, 0.0f), true);
		}
	}

	void HandleInputFlagsForCommonInput(InputFlags flags, Player* & player)
	{
		if (PressedOnce(flags, KEY_COMMA))
		{
			if (Rvn::frame.time_step > 0)
			{
				Rvn::frame.time_step -= 0.025;
			}
		}
		if (PressedOnce(flags, KEY_PERIOD))
		{
			if (Rvn::frame.time_step < 3)
			{
				Rvn::frame.time_step += 0.025;
			}
		}
		if (PressedOnce(flags, KEY_1))
		{
			Rvn::rm_buffer->Add("TIME STEP x0.05", 1000);
			Rvn::frame.time_step = 0.05;
		}
		if (PressedOnce(flags, KEY_2))
		{
			Rvn::rm_buffer->Add("TIME STEP x0.1", 1000);
			Rvn::frame.time_step = 0.1;
		}
		if (PressedOnce(flags, KEY_3))
		{
			Rvn::rm_buffer->Add("TIME STEP x0.3", 1000);
			Rvn::frame.time_step = 0.3;
		}
		if (PressedOnce(flags, KEY_4))
		{
			Rvn::rm_buffer->Add("TIME STEP x1.0", 1000);
			Rvn::frame.time_step = 1.0;
		}
		if (PressedOnce(flags, KEY_5))
		{
			Rvn::rm_buffer->Add("TIME STEP x2.0", 1000);
			Rvn::frame.time_step = 2.0;
		}
		if (flags.key_press & KEY_K)
		{
			player->Die();
		}
		if (PressedOnce(flags, KEY_F))
		{
			EditorState::ToggleProgramMode();
		}
		if (PressedOnce(flags, KEY_GRAVE_TICK))
		{
			StartConsoleMode();
		}
		if (flags.key_press & KEY_DELETE)
		{
			auto* GDC = GlobalDisplayConfig::Get();
			glfwSetWindowShouldClose(GDC->window, true);
		}
	}
}
