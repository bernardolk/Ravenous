#include "EditorInput.h"

#include <glfw3.h>

#include "EditorMain.h"
#include <imgui.h>

#include "console/console.h"
#include "editor/EditorState.h"
#include "tools/EditorTools.h"
#include "..\Game\Entities\Player.h"
#include "engine/Camera/Camera.h"
#include "engine/rvn.h"
#include "engine/collision/raycast.h"
#include "engine/io/display.h"
#include "engine/io/InputPhase.h"
#include "engine/serialization/sr_config.h"
#include "engine/io/input.h"
#include "engine/world/World.h"
#include "Reflection/Serialization.h"

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
		auto* Manager = RCameraManager::Get();
		auto* Camera = Manager->GetCurrentCamera();

		if (Pressed(Flags, NKeyInput::KeyLeftCtrl)) {
			Context.CtrlIsPressed = true;
		}
		
		if (Pressed(Flags, NKeyInput::KeyLeftCtrl) && PressedOnce(Flags, NKeyInput::KeyZ))
		{
			// snap mode controls the undo stack while it is active.
			if (!Context.SnapMode)
				Context.UndoStack.Undo();
			return;
		}

		if (Pressed(Flags, NKeyInput::KeyLeftCtrl) && PressedOnce(Flags, NKeyInput::KeyS))
		{
			EditorSave();
			return;
		}

		if (Pressed(Flags, NKeyInput::KeyLeftCtrl) && PressedOnce(Flags, NKeyInput::KeyY))
		{
			// snap mode controls the undo stack while it is active.
			if (!Context.SnapMode)
				Context.UndoStack.Redo();
			return;
		}

		if (PressedOnce(Flags, NKeyInput::KeyEsc))
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
		if (PressedOnce(Flags, NKeyInput::KeyDelete))
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
			if (PressedOnce(Flags, NKeyInput::KeyEnter))
			{
				SnapCommit();
			}
			if (PressedOnceExclusively(Flags, NKeyInput::KeyX))
			{
				if (Context.SnapAxis == 0) {
					Context.SnapCycle = (Context.SnapCycle + 1) % 3;
				}
				else
				{
					Context.SnapTrackedState.Apply();
					Context.SnapCycle = 0;
					Context.SnapAxis = 0;
				}

				if (Context.SnapReference.IsValid()) {
					SnapEntityToReference(*Context.EntityPanel.Entity);
				}
			}
			if (PressedOnceExclusively(Flags, NKeyInput::KeyY))
			{
				if (Context.SnapAxis == 1) {
					Context.SnapCycle = (Context.SnapCycle + 1) % 3;
				}
				else {
					Context.SnapTrackedState.Apply();
					Context.SnapCycle = 0;
					Context.SnapAxis = 1;
				}
				
				if (Context.SnapReference.IsValid()) {
					SnapEntityToReference(*Context.EntityPanel.Entity);
				}
			}
			if (PressedOnceExclusively(Flags, NKeyInput::KeyZ))
			{
				if (Context.SnapAxis == 2) {
					Context.SnapCycle = (Context.SnapCycle + 1) % 3;
				}
				else
				{
					Context.SnapTrackedState.Apply();
					Context.SnapCycle = 0;
					Context.SnapAxis = 2;
				}
				if (Context.SnapReference.IsValid()) {
					SnapEntityToReference(*Context.EntityPanel.Entity);
				}
			}
			if (PressedOnceExclusively(Flags, NKeyInput::KeyI))
			{
				Context.SnapInside = !Context.SnapInside;
				if (Context.SnapReference.IsValid()) {
					SnapEntityToReference(*Context.EntityPanel.Entity);
				}
			}
		}

		// --------------------
		// MOVE MODE SHORTCUTS
		// --------------------
		if (Context.MoveMode == true)
		{
			if (Pressed(Flags, NKeyInput::KeyX) && Pressed(Flags, NKeyInput::KeyZ))
			{
				Context.MoveAxis = 0;
			}
			if (PressedOnceExclusively(Flags, NKeyInput::KeyX))
			{
				Context.MoveAxis = 1;
			}
			if (PressedOnceExclusively(Flags, NKeyInput::KeyY))
			{
				Context.MoveAxis = 2;
			}
			if (PressedOnceExclusively(Flags, NKeyInput::KeyZ))
			{
				Context.MoveAxis = 3;
			}
			if (PressedOnceExclusively(Flags, NKeyInput::KeyM))
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
			if (PressedOnceExclusively(Flags, NKeyInput::KeyM))
			{
				Context.PlaceMode = false;
				Context.MoveMode = true;
				return;
			}
		}

		// -------------------
		// TRANSLATION TOOL
		// -------------------
		if (PressedOnce(Flags, NKeyInput::KeyT))
		{
			Context.UsingTranslationGizmo = true;
			Context.UsingRotationGizmo = false;
		}

		// -------------------
		// TRANSLATION TOOL
		// -------------------
		if (PressedOnce(Flags, NKeyInput::KeyR))
		{
			Context.UsingTranslationGizmo = false;
			Context.UsingRotationGizmo = true;
		}

		// ---------------
		// CLICK CONTROLS
		// ---------------

		// TODO: Refactor this whole thing: This checks for a CLICK then for modes to decide what todo.
		// I think this is not the best way to handle this, we should first check for MODE then for ACTION.
		// We can set things in context based on input Flags, OR use Flags directly.
		// Either way, there is code for handling clicks and etc both here and at editor_main which is confusing
		// and is, currently, causing some bugs in the editor.

		Context.MouseClick = false;
		Context.MouseDragging = false;

		auto* GII = GlobalInputInfo::Get();

		if (GII->MouseState & (uint16)NMouseInput::LeftButtonClick)
		{
			if (Context.SnapMode) {
				CheckSelectionToSnap();
			}
			
			else if (Context.MeasureMode) {
				CheckSelectionToMeasure(World);
			}
			
			else if (Context.LocateCoordsMode) {
				CheckSelectionToLocateCoords(World);
			}
			
			else if (Context.StretchMode) {
				CheckSelectionToStretch();
			}
			
			else if (Pressed(Flags, NKeyInput::KeyG)) {
				CheckSelectionToMoveEntity(World, Camera);
			}
			
			else
			{
				Context.MouseClick = true;
				if (Context.MoveMode || Context.PlaceMode) return;
				if (Context.EntityPanel.Active)
				{
					if (Context.SelectEntityAuxMode) return;
					if (Context.UsingTranslationGizmo && CheckSelectionToGrabEntityArrows(Camera)) return;
					if (Context.UsingRotationGizmo && CheckSelectionToGrabEntityRotationGizmo(Camera)) return;
				}
				CheckSelectionToOpenPanel(Player, World, Camera);
			}
		}

		else if (GII->MouseState & (uint16)NMouseInput::LeftButtonDragging)
		{
			Context.MouseDragging = true;
		}
		else if (GII->MouseState & (uint16)NMouseInput::LeftButtonHold)
		{
			Context.MouseDragging = true;
		}


		// -------------------------------
		// SPAWN PLAYER ON MOUSE POSITION
		// -------------------------------
		if (PressedOnce(Flags, NKeyInput::KeyC))
		{
			auto Pickray = CastPickray();
			auto Test = World->Raycast(Pickray);
			if (Test.Hit)
			{
				auto SurfacePoint = Test.GetPoint();
				Player->Position = SurfacePoint;
				Player->PlayerState = NPlayerState::Standing;
				Player->Velocity = vec3(0, 0, 0);
				Player->Update();
			}
		}

		// --------------------------------------------
		// CONTROL KEY USAGE BLOCKED FROM HERE ONWARDS
		// --------------------------------------------
		if (Pressed(Flags, NKeyInput::KeyLeftCtrl)) // this doesn't solve the full problem.
			return;

		// -------------------------
		// CAMERA MOVEMENT CONTROLS
		// -------------------------
		// @TODO: this sucks
		auto& Frame = RavenousEngine::GetFrame();
		auto* EditorCamera = Manager->GetEditorCamera();
		float CameraSpeed = Camera->Type == ThirdPerson ? length(Player->Velocity) * Frame.Duration : Frame.RealDuration * EditorCamera->Acceleration;

		if (Pressed(Flags, NKeyInput::KeyLeftShift))
		{
			CameraSpeed = CameraSpeed * 2;
		}

		if (Pressed(Flags, NKeyInput::KeyW))
		{
			Camera->Position += CameraSpeed * Camera->Front;
		}
		
		if (Pressed(Flags, NKeyInput::KeyA))
		{
			Camera->Position -= CameraSpeed * normalize(glm::cross(Camera->Front, Camera->Up));
		}
		
		if (Pressed(Flags, NKeyInput::KeyS) && !Pressed(Flags, NKeyInput::KeyLeftCtrl))
		{
			Camera->Position -= CameraSpeed * Camera->Front;
		}
		
		if (Pressed(Flags, NKeyInput::KeyD))
		{
			Camera->Position += CameraSpeed * normalize(glm::cross(Camera->Front, Camera->Up));
		}
		
		if (Pressed(Flags, NKeyInput::KeyQ))
		{
			Camera->Position -= CameraSpeed * Camera->Up;
		}
		
		if (Pressed(Flags, NKeyInput::KeyE))
		{
			Camera->Position += CameraSpeed * Camera->Up;
		}
		
		if (Pressed(Flags, NKeyInput::KeyO))
		{
			Manager->CameraLookAt(Camera, vec3(0.0f, 0.0f, 0.0f), true);
		}
	}

	void HandleInputFlagsForCommonInput(RInputFlags Flags, EPlayer* & Player)
	{
		auto& Frame = RavenousEngine::GetFrame();

		if (PressedOnce(Flags, NKeyInput::KeyComma))
		{
			if (Frame.TimeStep > 0) {
				Frame.TimeStep -= 0.025f;
			}
		}
		
		if (PressedOnce(Flags, NKeyInput::KeyPeriod))
		{
			if (Frame.TimeStep < 3) {
				Frame.TimeStep += 0.025f;
			}
		}
		
		if (PressedOnce(Flags, NKeyInput::Key1))
		{
			PrintEditorMsg("TIME STEP x0.05");
			Frame.TimeStep = 0.05f;
		}
		
		if (PressedOnce(Flags, NKeyInput::Key2))
		{
			PrintEditorMsg("TIME STEP x0.1");
			Frame.TimeStep = 0.1f;
		}
		
		if (PressedOnce(Flags, NKeyInput::Key3))
		{
			PrintEditorMsg("TIME STEP x0.3");
			Frame.TimeStep = 0.3f;
		}
		
		if (PressedOnce(Flags, NKeyInput::Key4))
		{
			PrintEditorMsg("TIME STEP x1.0");
			Frame.TimeStep = 1.0f;
		}
		
		if (PressedOnce(Flags, NKeyInput::Key5))
		{
			PrintEditorMsg("TIME STEP x2.0");
			Frame.TimeStep = 2.0f;
		}
		
		if (Pressed(Flags, NKeyInput::KeyK))
		{
			Player->Die();
		}
		
		if (PressedOnce(Flags, NKeyInput::KeyF))
		{
			REditorState::ToggleProgramMode();
		}
		
		if (PressedOnce(Flags, NKeyInput::KeyGraveTick))
		{
			StartConsoleMode();
		}
		
		if (Pressed(Flags, NKeyInput::KeyDelete))
		{
			if(auto& EntityPanel = GetContext()->EntityPanel; EntityPanel.Active) {
				EditorDeleteEntity(EntityPanel.Entity);
				EntityPanel.Active = false;
			}
		}

		if (Pressed(Flags, NKeyInput::KeyEnd))
		{
			auto* GDC = GlobalDisplayState::Get();
			glfwSetWindowShouldClose(GDC->GetWindow(), true);
		}
	}
}
