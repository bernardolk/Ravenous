#pragma once

#include "engine/core/core.h"
#include "editor/EditorContext.h"
#include "engine/collision/raycast.h"

namespace Editor
{
	void DeactivateEditorModes();
	bool CheckModesAreActive();
	void EditorEraseEntity(EEntity* entity);
	void EditorEraseLight(int index, string type, World* world);
	void UnhideEntities(World* world);

	// ----------
	// SNAP TOOL
	// ----------
	void ActivateSnapMode(EEntity* entity);
	void SnapEntityToReference(EEntity* entity);
	void CheckSelectionToSnap();
	void SnapCommit();

	// -------------
	// STRETCH TOOL
	// -------------
	void ActivateStretchMode(EEntity* entity);
	void StretchCommit();
	auto GetScaleAndPositionChange(EEntity* entity, float old_pos, float new_pos, float n);
	void StretchEntityToReference(EEntity* entity);
	void CheckSelectionToStretch();

	// -------------
	// MEASURE TOOL
	// -------------
	void ActivateMeasureMode(u8 axis);
	void CheckSelectionToMeasure(const World* world);

	// ------------------------
	// LOCATE COORDINATES MODE
	// ------------------------
	void ActivateLocateCoordsMode();
	auto CheckSelectionToLocateCoords(const World* world) -> void;

	// -------------
	// > MOVE TOOLS 
	// -------------
	void PlaceEntity(World* world);
	RaycastTest TestRayAgainstEntitySupportPlane(u16 move_axis, EEntity* entity);

	// --------------
	// >> PLACE MODE
	// --------------
	void ActivatePlaceMode(EEntity* entity);
	void SelectEntityPlacingWithMouseMove(EEntity* entity, const World* world);
	
	// -------------
	// >> MOVE MODE
	// -------------
	void ActivateMoveMode(EEntity* entity);
	void MoveEntityWithMouse(EEntity* entity);
	
	// -------------------------
	// >> MOVE ENTITY BY ARROWS
	// -------------------------
	void ActivateMoveEntityByArrow(u8 move_axis);
	void MoveEntityByArrows(EEntity* entity);

	// ----------------
	// MOVE LIGHT TOOL
	// ----------------
	// @todo: This will DISAPPEAR after lights become entities!
	//       We need to provide entity rights to lights too! revolution now!

	void MoveLightWithMouse(std::string type, int index, World* world);
	void ActivateMoveLightMode(std::string type, int index);
	void PlaceLight(std::string type, int index);
	void PlaceLight();
	void OpenLightsPanel(std::string type, int index, bool focus_tab); //fwd
	
	// ---------------------
	// > ROTATE ENTITY TOOL
	// ---------------------
	void ActivateRotateEntityWithMouse(u8 move_axis);
	float MouseOffsetToAngularOffset(float mouse_offset);
	void RotateEntityWithMouse(EEntity* entity);

	// ------------------
	// SCALE ENTITY TOOL
	// ------------------
	void ScaleEntityWithMouse(EEntity* entity);

	// -----------------------
	// SELECT ENTITY AUX TOOL
	// -----------------------
	// used in entity panel to select other entity to attribute 1 to 1 relationships
	void ActivateSelectEntityAuxTool(EEntity** entity_slot, EdToolCallback callback = EdToolCallback_NoCallback, EdToolCallbackArgs args = EdToolCallbackArgs{});

	// -------------
	// MISCELANEOUS
	// -------------
	// void CheckForAssetChanges();
	void RenderAabbBoundaries(EEntity* entity);
}
