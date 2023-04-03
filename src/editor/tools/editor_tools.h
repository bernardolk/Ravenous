#pragma once

#include "engine/core/core.h"
#include "editor/editor_context.h"
#include "engine/collision/raycast.h"

namespace Editor
{
	void DeactivateEditorModes();
	bool CheckModesAreActive();
	void EditorEraseEntity(Entity* entity);
	void EditorEraseLight(int index, string type, T_World* world);
	void UnhideEntities(const World* world);

	// ----------
	// SNAP TOOL
	// ----------
	void ActivateSnapMode(Entity* entity);
	void SnapEntityToReference(Entity* entity);
	void CheckSelectionToSnap();
	void SnapCommit();

	// -------------
	// STRETCH TOOL
	// -------------
	void ActivateStretchMode(Entity* entity);
	void StretchCommit();
	auto GetScaleAndPositionChange(Entity* entity, float old_pos, float new_pos, float n);
	void StretchEntityToReference(Entity* entity);
	void CheckSelectionToStretch();

	// -------------
	// MEASURE TOOL
	// -------------
	void ActivateMeasureMode(u8 axis);
	void CheckSelectionToMeasure(const T_World* world);

	// ------------------------
	// LOCATE COORDINATES MODE
	// ------------------------
	void ActivateLocateCoordsMode();
	auto CheckSelectionToLocateCoords(const T_World* world) -> void;

	// -------------
	// > MOVE TOOLS 
	// -------------
	void PlaceEntity(T_World* world);
	RaycastTest TestRayAgainstEntitySupportPlane(u16 move_axis, E_Entity* entity);

	// --------------
	// >> PLACE MODE
	// --------------
	void ActivatePlaceMode(E_Entity* entity);
	void SelectEntityPlacingWithMouseMove(E_Entity* entity, const T_World* world);
	
	// -------------
	// >> MOVE MODE
	// -------------
	void ActivateMoveMode(E_Entity* entity);
	void MoveEntityWithMouse(E_Entity* entity);
	
	// -------------------------
	// >> MOVE ENTITY BY ARROWS
	// -------------------------
	void ActivateMoveEntityByArrow(u8 move_axis);
	void MoveEntityByArrows(E_Entity* entity);

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
	void RotateEntityWithMouse(E_Entity* entity);

	// ------------------
	// SCALE ENTITY TOOL
	// ------------------
	void ScaleEntityWithMouse(E_Entity* entity);

	// -----------------------
	// SELECT ENTITY AUX TOOL
	// -----------------------
	// used in entity panel to select other entity to attribute 1 to 1 relationships
	void ActivateSelectEntityAuxTool(E_Entity** entity_slot, EdToolCallback callback = EdToolCallback_NoCallback, EdToolCallbackArgs args = EdToolCallbackArgs{});

	// -------------
	// MISCELANEOUS
	// -------------
	// void CheckForAssetChanges();
	void RenderAabbBoundaries(E_Entity* entity);
}
