#include "EditorInput.h"

#include <glfw3.h>

#include "editor.h"
#include <imgui.h>

#include "console/console.h"
#include "editor/EditorState.h"
#include "tools/EditorTools.h"
#include "game/entities/EPlayer.h"
#include "engine/camera/camera.h"
#include "engine/rvn.h"
#include "engine/collision/raycast.h"
#include "engine/io/display.h"
#include "engine/io/InputPhase.h"
#include "engine/serialization/sr_config.h"
#include "engine/serialization/sr_world.h"
#include "engine/io/input.h"
#include "engine/world/World.h"

namespace Editor
{
	void HandleInputFlagsForEditorMode(RInputFlags Flags, RWorld* World)
	{
		// ------------------------
		// EDITOR EDITING COMMANDS
		// ------------------------
		// commands that return once detected,
		// not allowing for more than one at a time
		// to be issued.
		auto& Context = *Editor::GetContext();
		auto* Player = EPlayer::Get();
		auto& ProgramConfig = *ProgramConfig::Get();
		auto* Manager = RCameraManager::Get();
		auto* Camera = Manager->GetCurrentCamera();

		if (Pressed(flags, KEY_LEFT_CTRL) && PressedOnce(flags, KEY_Z))
		{
			// snap mode controls the undo stack while it is active.
			if (!Context.SnapMode)
				Context.UndoStack.Undo();
			return;
		}

		if (Pressed(flags, KEY_LEFT_CTRL) && PressedOnce(flags, KEY_S))
		{
			// save scene
			Player->checkpoint_pos = Player->Position;
			WorldSerializer::SaveToFile();
			// set scene
			ProgramConfig.InitialScene = RWorld::Get()->SceneName;
			ConfigSerializer::Save(ProgramConfig);
			Rvn::RmBuffer->Add("world state saved", 1200);
			return;
		}

		if (Pressed(flags, KEY_LEFT_CTRL) && PressedOnce(flags, KEY_Y))
		{
			// snap mode controls the undo stack while it is active.
			if (!Context.SnapMode)
				Context.UndoStack.Redo();
			return;
		}

		if (PressedOnce(flags, KEY_ESC))
		{
			if (Editor::CheckModesAreActive())
				DeactivateEditorModes();
			else if (Context.EntityPanel.Active)
				Context.EntityPanel.Active = false;
			else if (Context.WorldPanel.Active)
				Context.WorldPanel.Active = false;
			else if (Context.LightsPanel.Active)
				Context.LightsPanel.Active = false;
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
		if (Context.SnapMode == true)
		{
			if (PressedOnce(flags, KEY_ENTER))
			{
				SnapCommit();
			}
			if (PressedOnly(flags, KEY_X))
			{
				if (Context.SnapAxis == 0)
					Context.SnapCycle = (Context.SnapCycle + 1) % 3;
				else
				{
					ApplyState(Context.SnapTrackedState);
					Context.SnapCycle = 0;
					Context.SnapAxis = 0;
				}
				if (Context.SnapReference != nullptr)
					SnapEntityToReference(Context.EntityPanel.Entity);
			}
			if (PressedOnly(flags, KEY_Y))
			{
				if (Context.SnapAxis == 1)
					Context.SnapCycle = (Context.SnapCycle + 1) % 3;
				else
				{
					ApplyState(Context.SnapTrackedState);
					Context.SnapCycle = 0;
					Context.SnapAxis = 1;
				}
				if (Context.SnapReference != nullptr)
					SnapEntityToReference(Context.EntityPanel.Entity);
			}
			if (PressedOnly(flags, KEY_Z))
			{
				if (Context.SnapAxis == 2)
					Context.SnapCycle = (Context.SnapCycle + 1) % 3;
				else
				{
					ApplyState(Context.SnapTrackedState);
					Context.SnapCycle = 0;
					Context.SnapAxis = 2;
				}
				if (Context.SnapReference != nullptr)
					SnapEntityToReference(Context.EntityPanel.Entity);
			}
			if (PressedOnly(flags, KEY_I))
			{
				Context.SnapInside = !Context.SnapInside;
				if (Context.SnapReference != nullptr)
					SnapEntityToReference(Context.EntityPanel.Entity);
			}
		}

		// --------------------
		// MOVE MODE SHORTCUTS
		// --------------------
		if (Context.MoveMode == true)
		{
			if (Pressed(flags, KEY_X) && Pressed(flags, KEY_Z))
			{
				Context.MoveAxis = 0;
			}
			if (PressedOnly(flags, KEY_X))
			{
				Context.MoveAxis = 1;
			}
			if (PressedOnly(flags, KEY_Y))
			{
				Context.MoveAxis = 2;
			}
			if (PressedOnly(flags, KEY_Z))
			{
				Context.MoveAxis = 3;
			}
			if (PressedOnly(flags, KEY_M))
			{
				Context.MoveMode = false;
				Context.PlaceMode = true;
				return;
			}
		}

		// ---------------------
		// PLACE MODE SHORTCUTS
		// ---------------------
		if (Context.PlaceMode == true)
		{
			if (PressedOnly(flags, KEY_M))
			{
				Context.PlaceMode = false;
				Context.MoveMode = true;
				return;
			}
		}

		// -------------------
		// CAMERA TYPE TOGGLE
		// -------------------
		if (PressedOnce(flags, KEY_T))
		{
			// toggle camera type
			if (Camera->Type == FreeRoam)
				Manager->SetCameraToThirdPerson();
			else if (Camera->Type == ThirdPerson)
				Manager->SetCameraToFreeRoam();
		}

		// ---------------
		// CLICK CONTROLS
		// ---------------

		// TODO: Refactor this whole thing: This checks for a CLICK then for modes to decide what todo.
		// I think this is not the best way to handle this, we should first check for MODE then for ACTION.
		// We can set things in context based on input flags, OR use flags directly.
		// Either way, there is code for handling clicks and etc both here and at editor_main which is confusing
		// and is, currently, causing some bugs in the editor.

		Context.MouseClick = false;
		Context.MouseDragging = false;

		auto* GII = GlobalInputInfo::Get();

		if (GII->MouseState & MOUSE_LB_CLICK)
		{
			if (Context.SnapMode)
			{
				CheckSelectionToSnap();
			}
			else if (Context.MeasureMode)
			{
				CheckSelectionToMeasure(World);
			}
			else if (Context.LocateCoordsMode)
			{
				CheckSelectionToLocateCoords(World);
			}
			else if (Context.StretchMode)
			{
				CheckSelectionToStretch();
			}
			else if (flags.key_press & KEY_G)
			{
				CheckSelectionToMoveEntity(World, Camera);
			}
			else
			{
				Context.MouseClick = true;

				if (Context.EntityPanel.Active)
				{
					if (Context.SelectEntityAuxMode)
						return;
					if (CheckSelectionToGrabEntityArrows(Camera))
						return;
					if (CheckSelectionToGrabEntityRotationGizmo(Camera))
						return;
				}

				if (Context.MoveMode || Context.PlaceMode)
					return;

				CheckSelectionToOpenPanel(Player, World, Camera);
			}
		}

		else if (GII->MouseState & MOUSE_LB_DRAGGING)
		{
			Context.MouseDragging = true;
		}
		else if (GII->MouseState & MOUSE_LB_HOLD)
		{
			Context.MouseDragging = true;
		}


		// -------------------------------
		// SPAWN PLAYER ON MOUSE POSITION
		// -------------------------------
		if (PressedOnce(flags, KEY_C))
		{
			auto Pickray = CastPickray(Camera, GII->MouseCoords.X, GII->MouseCoords.Y);
			auto Test = World->Raycast(Pickray);
			if (Test.Hit)
			{
				auto SurfacePoint = CL_GetPointFromDetection(pickray, test);
				Player->Position = surface_point;
				Player->player_state = NPlayerState::Standing;
				Player->Velocity = vec3(0, 0, 0);
				Player->Update();
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
		auto& Frame = RavenousEngine::GetFrame();
		auto* EditorCamera = Manager->GetEditorCamera();
		float CameraSpeed = Camera->Type == ThirdPerson ? length(Player->Velocity) * Frame.Duration : Frame.RealDuration * EditorCamera->Acceleration;

		if (flags.key_press & KEY_LEFT_SHIFT)
		{
			CameraSpeed = CameraSpeed * 2;
		}

		if (flags.key_press & KEY_W)
		{
			Camera->Position += CameraSpeed * Camera->Front;
		}
		if (flags.key_press & KEY_A)
		{
			// @TODO: this sucks too
			if (Camera->Type == FreeRoam)
				camera->position -= camera_speed * normalize(glm::cross(camera->front, camera->up));
			else if (Camera->Type == ThirdPerson)
				Camera->OrbitalAngle -= 0.025;
		}
		if (Pressed(flags, KEY_S))
		{
			Camera->Position -= CameraSpeed * Camera->Front;
		}
		if (flags.key_press & KEY_D)
		{
			if (Camera->Type == FreeRoam)
				camera->position += camera_speed * normalize(glm::cross(camera->front, camera->up));
			else if (Camera->Type == ThirdPerson)
				Camera->OrbitalAngle += 0.025;
		}
		if (flags.key_press & KEY_Q)
		{
			Camera->Position -= CameraSpeed * Camera->Up;
		}
		if (flags.key_press & KEY_E)
		{
			Camera->Position += CameraSpeed * Camera->Up;
		}
		if (flags.key_press & KEY_O)
		{
			Manager->CameraLookAt(Camera, vec3(0.0f, 0.0f, 0.0f), true);
		}
	}

	void HandleInputFlagsForCommonInput(RInputFlags Flags, EPlayer* & Player)
	{
		auto& Frame = RavenousEngine::GetFrame();

		if (PressedOnce(flags, KEY_COMMA))
		{
			if (Frame.TimeStep > 0)
			{
				Frame.TimeStep -= 0.025;
			}
		}
		if (PressedOnce(flags, KEY_PERIOD))
		{
			if (Frame.TimeStep < 3)
			{
				Frame.TimeStep += 0.025;
			}
		}
		if (PressedOnce(flags, KEY_1))
		{
			Rvn::RmBuffer->Add("TIME STEP x0.05", 1000);
			Frame.TimeStep = 0.05;
		}
		if (PressedOnce(flags, KEY_2))
		{
			Rvn::RmBuffer->Add("TIME STEP x0.1", 1000);
			Frame.TimeStep = 0.1;
		}
		if (PressedOnce(flags, KEY_3))
		{
			Rvn::RmBuffer->Add("TIME STEP x0.3", 1000);
			Frame.TimeStep = 0.3;
		}
		if (PressedOnce(flags, KEY_4))
		{
			Rvn::RmBuffer->Add("TIME STEP x1.0", 1000);
			Frame.TimeStep = 1.0;
		}
		if (PressedOnce(flags, KEY_5))
		{
			Rvn::RmBuffer->Add("TIME STEP x2.0", 1000);
			Frame.TimeStep = 2.0;
		}
		if (flags.key_press & KEY_K)
		{
			Player->Die();
		}
		if (PressedOnce(flags, KEY_F))
		{
			REditorState::ToggleProgramMode();
		}
		if (PressedOnce(flags, KEY_GRAVE_TICK))
		{
			StartConsoleMode();
		}
		if (flags.key_press & KEY_DELETE)
		{
			auto* GDC = GlobalDisplayState::Get();
			glfwSetWindowShouldClose(GDC->GetWindow(), true);
		}
	}
}
