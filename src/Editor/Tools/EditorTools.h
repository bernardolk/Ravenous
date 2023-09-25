#pragma once

#include "engine/core/core.h"
#include "editor/EditorContext.h"
#include "engine/collision/raycast.h"

namespace Editor
{
	void DeactivateEditorModes();
	bool CheckModesAreActive();
	void EditorEraseEntity(EEntity* Entity);
	void EditorEraseLight(int Index, string Type, RWorld* World);
	void UnhideEntities(RWorld* World);

	// ----------
	// SNAP TOOL
	// ----------
	void ActivateSnapMode(EEntity* Entity);
	void SnapEntityToReference(EEntity* Entity);
	void CheckSelectionToSnap();
	void SnapCommit();

	// -------------
	// STRETCH TOOL
	// -------------
	void ActivateStretchMode(EEntity* Entity);
	void StretchCommit();
	auto GetScaleAndPositionChange(EEntity* Entity, float OldPos, float NewPos, float N);
	void StretchEntityToReference(EEntity* Entity);
	void CheckSelectionToStretch();

	// -------------
	// MEASURE TOOL
	// -------------
	void ActivateMeasureMode(uint8 Axis);
	void CheckSelectionToMeasure(const RWorld* World);

	// ------------------------
	// LOCATE COORDINATES MODE
	// ------------------------
	void ActivateLocateCoordsMode();
	auto CheckSelectionToLocateCoords(const RWorld* World) -> void;

	// -------------
	// > MOVE TOOLS 
	// -------------
	void PlaceEntity(RWorld* World);
	RRaycastTest TestRayAgainstEntitySupportPlane(uint16 MoveAxis, EEntity* Entity);

	// --------------
	// >> PLACE MODE
	// --------------
	void ActivatePlaceMode(EEntity* Entity);
	void SelectEntityPlacingWithMouseMove(EEntity* Entity, const RWorld* World);

	// -------------
	// >> MOVE MODE
	// -------------
	void ActivateMoveMode(EEntity* Entity);
	void MoveEntityWithMouse(EEntity* Entity);

	// -------------------------
	// >> MOVE ENTITY BY ARROWS
	// -------------------------
	void ActivateMoveEntityByArrow(uint8 MoveAxis);
	void MoveEntityByArrows(EEntity* Entity);

	// ----------------
	// MOVE LIGHT TOOL
	// ----------------
	// @todo: This will DISAPPEAR after lights become entities!
	//       We need to provide entity rights to lights too! revolution now!

	void MoveLightWithMouse(std::string Type, int Index, RWorld* World);
	void ActivateMoveLightMode(std::string Type, int Index);
	void PlaceLight(std::string Type, int Index);
	void PlaceLight();
	void OpenLightsPanel(std::string Type, int Index, bool FocusTab); //fwd

	// ---------------------
	// > ROTATE ENTITY TOOL
	// ---------------------
	void ActivateRotateEntityWithMouse(uint8 MoveAxis);
	float MouseOffsetToAngularOffset(float MouseOffset);
	void RotateEntityWithMouse(EEntity* Entity);

	// ------------------
	// SCALE ENTITY TOOL
	// ------------------
	void ScaleEntityWithMouse(EEntity* Entity);

	// -----------------------
	// SELECT ENTITY AUX TOOL
	// -----------------------
	// used in entity panel to select other entity to attribute 1 to 1 relationships
	void ActivateSelectEntityAuxTool(EEntity** EntitySlot, NEdToolCallback Callback = EdToolCallback_NoCallback, REditorToolCallbackArgs Args = REditorToolCallbackArgs{});

	// -------------
	// MISCELANEOUS
	// -------------
	// void CheckForAssetChanges();
	void RenderAabbBoundaries(EEntity* Entity);
}
