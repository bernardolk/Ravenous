#include "editor_input.h"
#include "editor.h"
#include <imgui.h>
#include "tools/editor_tools.h"
#include "player.h"
#include "engine/camera.h"
#include "engine/rvn.h"
#include "engine/collision/raycast.h"
#include "engine/loop/input_phase.h"
#include "engine/serialization/sr_config.h"
#include "engine/serialization/sr_world.h"
#include "engine/world/scene_manager.h"
#include "engine/io/input.h"
#include "engine/world/world.h"

namespace Editor
{
	void HandleInputFlags(InputFlags flags, World* world, Camera* camera)
	{
		// ------------------------
		// EDITOR EDITING COMMANDS
		// ------------------------
		// commands that return once detected,
		// not allowing for more than one at a time
		// to be issued.
		auto* GSI = GlobalSceneInfo::Get();
		auto& context = *Editor::GetContext();
		auto* player = Player::Get();
		auto& program_config = *ProgramConfig::Get();

		if(Pressed(flags, KEY_LEFT_CTRL) && PressedOnce(flags, KEY_Z))
		{
			// snap mode controls the undo stack while it is active.
			if(!context.snap_mode)
				context.undo_stack.Undo();
			return;
		}

		if(Pressed(flags, KEY_LEFT_CTRL) && PressedOnce(flags, KEY_S))
		{
			// save scene
			player->checkpoint_pos = player->entity_ptr->position;
			WorldSerializer::SaveToFile();
			// set scene
			program_config.initial_scene = GSI->scene_name;
			ConfigSerializer::save(program_config);
			Rvn::rm_buffer->Add("world state saved", 1200);
			return;
		}

		if(Pressed(flags, KEY_LEFT_CTRL) && PressedOnce(flags, KEY_Y))
		{
			// snap mode controls the undo stack while it is active.
			if(!context.snap_mode)
				context.undo_stack.Redo();
			return;
		}

		if(PressedOnce(flags, KEY_ESC))
		{
			if(Editor::check_modes_are_active())
				deactivate_editor_modes();
			else if(context.entity_panel.active)
				context.entity_panel.active = false;
			else if(context.world_panel.active)
				context.world_panel.active = false;
			else if(context.lights_panel.active)
				context.lights_panel.active = false;
			return;
		}

		// TODO: abstract call to not have to import imgui.h
		if(ImGui::GetIO().WantCaptureKeyboard)
			return;

		if(PressedOnce(flags, KEY_DELETE))
		{
			if(context.entity_panel.active && context.entity_panel.focused)
			{
				context.entity_panel.active = false;
				editor_erase_entity(context.entity_panel.entity);
				return;
			}
			if(context.lights_panel.active && context.lights_panel.focused)
			{
				if(context.lights_panel.selected_light > -1)
				{
					editor_erase_light(context.lights_panel.selected_light, context.lights_panel.selected_light_type, world);
				}
				return;
			}
		}

		// ------------------------------------
		// TOOLS / CAMERA / CHARACTER CONTROLS
		// ------------------------------------

		// --------------------
		// SNAP MODE SHORTCUTS
		// --------------------
		if(context.snap_mode == true)
		{
			if(PressedOnce(flags, KEY_ENTER))
			{
				snap_commit();
			}
			if(PressedOnly(flags, KEY_X))
			{
				if(context.snap_axis == 0)
					context.snap_cycle = (context.snap_cycle + 1) % 3;
				else
				{
					apply_state(context.snap_tracked_state);
					context.snap_cycle = 0;
					context.snap_axis = 0;
				}
				if(context.snap_reference != nullptr)
					snap_entity_to_reference(context.entity_panel.entity);
			}
			if(PressedOnly(flags, KEY_Y))
			{
				if(context.snap_axis == 1)
					context.snap_cycle = (context.snap_cycle + 1) % 3;
				else
				{
					apply_state(context.snap_tracked_state);
					context.snap_cycle = 0;
					context.snap_axis = 1;
				}
				if(context.snap_reference != nullptr)
					snap_entity_to_reference(context.entity_panel.entity);
			}
			if(PressedOnly(flags, KEY_Z))
			{
				if(context.snap_axis == 2)
					context.snap_cycle = (context.snap_cycle + 1) % 3;
				else
				{
					apply_state(context.snap_tracked_state);
					context.snap_cycle = 0;
					context.snap_axis = 2;
				}
				if(context.snap_reference != nullptr)
					snap_entity_to_reference(context.entity_panel.entity);
			}
			if(PressedOnly(flags, KEY_I))
			{
				context.snap_inside = !context.snap_inside;
				if(context.snap_reference != nullptr)
					snap_entity_to_reference(context.entity_panel.entity);
			}
		}

		// --------------------
		// MOVE MODE SHORTCUTS
		// --------------------
		if(context.move_mode == true)
		{
			if(Pressed(flags, KEY_X) && Pressed(flags, KEY_Z))
			{
				context.move_axis = 0;
			}
			if(PressedOnly(flags, KEY_X))
			{
				context.move_axis = 1;
			}
			if(PressedOnly(flags, KEY_Y))
			{
				context.move_axis = 2;
			}
			if(PressedOnly(flags, KEY_Z))
			{
				context.move_axis = 3;
			}
			if(PressedOnly(flags, KEY_M))
			{
				context.move_mode = false;
				context.place_mode = true;
				return;
			}
		}

		// ---------------------
		// PLACE MODE SHORTCUTS
		// ---------------------
		if(context.place_mode == true)
		{
			if(PressedOnly(flags, KEY_M))
			{
				context.place_mode = false;
				context.move_mode = true;
				return;
			}
		}

		// -------------------
		// CAMERA TYPE TOGGLE
		// -------------------
		if(PressedOnce(flags, KEY_T))
		{
			// toggle camera type
			if(GSI->camera->type == FREE_ROAM)
				set_camera_to_third_person(GSI->camera);
			else if(GSI->camera->type == THIRD_PERSON)
				set_camera_to_free_roam(GSI->camera);
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

		if(GII->mouse_state & MOUSE_LB_CLICK)
		{
			std::cout << "CLICK COUNT\n";
			if(context.snap_mode)
			{
				check_selection_to_snap();
			}
			else if(context.measure_mode)
			{
				check_selection_to_measure(world);
			}
			else if(context.locate_coords_mode)
			{
				check_selection_to_locate_coords(world);
			}
			else if(context.stretch_mode)
			{
				check_selection_to_stretch();
			}
			else if(flags.key_press & KEY_G)
			{
				CheckSelectionToMoveEntity(world, camera);
			}
			else
			{
				context.mouse_click = true;

				if(context.entity_panel.active)
				{
					if(context.select_entity_aux_mode)
						return;
					if(CheckSelectionToGrabEntityArrows(camera))
						return;
					if(CheckSelectionToGrabEntityRotationGizmo(camera))
						return;
				}

				if(context.move_mode || context.place_mode)
					return;

				CheckSelectionToOpenPanel(player, world, camera);
			}
		}

		else if(GII->mouse_state & MOUSE_LB_DRAGGING)
		{
			context.mouse_dragging = true;
		}
		else if(GII->mouse_state & MOUSE_LB_HOLD)
		{
			context.mouse_dragging = true;
		}


		// -------------------------------
		// SPAWN PLAYER ON MOUSE POSITION
		// -------------------------------
		if(PressedOnce(flags, KEY_C))
		{
			auto pickray = cast_pickray(GSI->camera, GII->mouse_coords.x, GII->mouse_coords.y);
			auto test = world->Raycast(pickray, player->entity_ptr);
			if(test.hit)
			{
				auto surface_point = point_from_detection(pickray, test);
				player->entity_ptr->position = surface_point;
				player->player_state = PLAYER_STATE_STANDING;
				player->standing_entity_ptr = test.entity;
				player->entity_ptr->velocity = vec3(0, 0, 0);
				player->Update(world);
			}
		}

		// --------------------------------------------
		// CONTROL KEY USAGE BLOCKED FROM HERE ONWARDS
		// --------------------------------------------
		if(Pressed(flags, KEY_LEFT_CTRL)) // this doesn't solve the full problem.
			return;

		// -------------------------
		// CAMERA MOVEMENT CONTROLS
		// -------------------------
		// @TODO: this sucks
		auto* editor_camera = GlobalSceneInfo::GetEditorCam();
		float camera_speed =
			GSI->camera->type == THIRD_PERSON ?
			player->speed * Rvn::frame.duration :
			Rvn::frame.real_duration * editor_camera->acceleration;

		if(flags.key_press & KEY_LEFT_SHIFT)
		{
			camera_speed = camera_speed * 2;
		}

		if(flags.key_press & KEY_W)
		{
			GSI->camera->position += camera_speed * GSI->camera->front;
		}
		if(flags.key_press & KEY_A)
		{
			// @TODO: this sucks too
			if(GSI->camera->type == FREE_ROAM)
				GSI->camera->position -= camera_speed * normalize(glm::cross(GSI->camera->front, GSI->camera->up));
			else if(GSI->camera->type == THIRD_PERSON)
				GSI->camera->orbital_angle -= 0.025;
		}
		if(Pressed(flags, KEY_S))
		{
			GSI->camera->position -= camera_speed * GSI->camera->front;
		}
		if(flags.key_press & KEY_D)
		{
			if(GSI->camera->type == FREE_ROAM)
				GSI->camera->position += camera_speed * normalize(glm::cross(GSI->camera->front, GSI->camera->up));
			else if(GSI->camera->type == THIRD_PERSON)
				GSI->camera->orbital_angle += 0.025;
		}
		if(flags.key_press & KEY_Q)
		{
			GSI->camera->position -= camera_speed * GSI->camera->up;
		}
		if(flags.key_press & KEY_E)
		{
			GSI->camera->position += camera_speed * GSI->camera->up;
		}
		if(flags.key_press & KEY_O)
		{
			camera_look_at(GSI->camera, vec3(0.0f, 0.0f, 0.0f), true);
		}
	}
}
