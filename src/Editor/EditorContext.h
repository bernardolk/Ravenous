#pragma once

#include "engine/core/core.h"
#include "editor/EditorPanelContexts.h"
#include "editor/EditorUndo.h"
#include "EntityState.h"
#include "Engine/Geometry/Plane.h"

namespace Editor
{
	enum NEdToolCallback
	{
		EdToolCallback_NoCallback           = 0,
		EdToolCallback_EntityManagerSetType = 1,
	};

	struct REditorToolCallbackArgs
	{
		//@entityptr
		EEntity* Entity;
	};

	struct REditorContext
	{
		static REditorContext* Get() 
		{ 
			static REditorContext Instance{};
			return &Instance;
		}

		void RemoveFromDeletedEntityList(RUUID ID);
		
		struct ImGuiStyle* ImStyle = nullptr;

		// scene tracking
		string LastFrameScene;

		// undo stack
		RUndoStack UndoStack;

		// Deletion Log
		vector<RUUID> DeletionLog;

		// panels
		RSceneObjectsPanelContext SceneObjectsPanel;
		REntityPanelContext EntityPanel;
		RPlayerPanelContext PlayerPanel;
		RWorldPanelContext WorldPanel;
		RPalettePanelContext PalettePanel;
		RLightsPanelContext LightsPanel;
		RCollisionLogPanelContext CollisionLogPanel;
		RInputRecorderPanelContext InputRecorderPanel;

		// toolbar
		bool ToolbarActive = true;

		// general mode controls
		bool MouseClick = false;
		bool MouseDragging = false;

		EHandle<EEntity> SelectedEntity;

		// move mode
		bool MoveMode = false;
		bool ScaleOnDrop = false;
		uint8 MoveAxis = 0;

		// move entity by arrows
		bool MoveEntityByArrows = false;
		RPlane MoveEntityByArrowsReferencePlane;
		vec3 MoveEntityByArrowsRefPoint;
		
		// rotate entity with mouse
		bool RotateEntityWithMouse = false;
		vec2 RotateEntityWithMouseMouseCoordsRef = vec2(0);

		// place mode
		bool PlaceMode = false;

		// move light
		// @todo: will disappear!
		string SelectedLightType;
		int SelectedLight = -1;

		// scale mode
		bool ScaleEntityWithMouse = false;

		// measure mode
		bool MeasureMode = false;
		uint8 MeasureAxis = 0; // x,y,z == 0,1,2
		vec3 MeasureFrom;
		bool FirstPointFound = false;
		bool SecondPointFound = false;
		float MeasureTo;

		// locate coordinates mode
		bool LocateCoordsMode = false;
		bool LocateCoordsFoundPoint = false;
		vec3 LocateCoordsPosition;

		// snap mode
		bool SnapMode = false;
		uint8 SnapCycle = 0;
		uint8 SnapAxis = 1;
		bool SnapInside = false;
		
		EHandle<EEntity> SnapReference;
		REntityState SnapTrackedState;

		// stretch mode
		bool StretchMode = false;

		// select entity aux tool
		bool SelectEntityAuxMode = false;

		//@entityptr
		EEntity** SelectEntityAuxModeEntitySlot = nullptr;
		NEdToolCallback SelectEntityAuxModeCallback = EdToolCallback_NoCallback;
		REditorToolCallbackArgs SelectEntityAuxModeCallbackArgs = REditorToolCallbackArgs{};

		// show things 
		bool ShowEventTriggers = false;
		bool ShowWorldCells = false;
		bool ShowLightbulbs = true;

		// gizmos
		//@entityptr
		EEntity* TriAxis[3];
		EEntity* TriAxisLetters[3];

		bool UsingTranslationGizmo = true;
		bool UsingRotationGizmo;

		bool CtrlIsPressed = false;
		
		// debug options
		bool DebugLedgeDetection = false;
		
		bool bGizmoPositionsDirty = false;
	};

	inline REditorContext* GetContext() { return REditorContext::Get(); }

}
