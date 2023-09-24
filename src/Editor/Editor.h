#pragma once

#include "engine/core/core.h"
#include "EditorContext.h"
#include "engine/rvn.h"

#define WITH_EDITOR 1

namespace Editor
{
	void StartDearImguiFrame();
	void EndDearImguiFrame();

	const static std::string EditorAssets = Paths::Project + "/assets/editor/";

	constexpr static float TriaxisScreenposX = -1.80f;
	constexpr static float TriaxisScreenposY = -1.80f;


	void Initialize();
	void Update(EPlayer* player, RWorld* world, RCamera* camera);
	void Render(EPlayer* player, RWorld* world, RCamera* camera);
	void Terminate();

	void UpdateTriaxisGizmo();
	void CheckSelectionToOpenPanel(EPlayer* player, RWorld* world, RCamera* camera);
	bool CheckSelectionToGrabEntityArrows(RCamera* camera);
	bool CheckSelectionToGrabEntityRotationGizmo(RCamera* camera);
	void CheckSelectionToMoveEntity(RWorld* world, RCamera* camera);
	void CheckSelectionToSelectRelatedEntity(RWorld* world, RCamera* camera);

	void RenderTextOverlay(EPlayer* player, RCamera* camera);
	void RenderEventTriggers(RCamera* camera, RWorld* world);
	void UpdateEntityControlArrows(REntityPanelContext* panel);
	void RenderEntityControlArrows(REntityPanelContext* panel, RWorld* world, RCamera* camera);
	void RenderEntityRotationGizmo(REntityPanelContext* panel, RWorld* world, RCamera* camera);
	void UpdateEntityRotationGizmo(REntityPanelContext* panel);
	void RenderEntityMeshNormals(REntityPanelContext* panel);
	float GetGizmoScalingFactor(EEntity* entity, float min, float max);
	void RenderWorldCells(RCamera* camera, RWorld* world);
	void RenderLightbulbs(RCamera* camera, RWorld* world);
	void StartDearImguiFrame();
	void EndDearImguiFrame();

	inline REditorContext* GetContext() { return REditorContext::Get(); }
}
