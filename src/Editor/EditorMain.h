#pragma once

#include "engine/core/core.h"
#include "EditorContext.h"
#include "engine/rvn.h"

namespace Editor
{
	void StartDearImguiFrame();
	void EndDearImguiFrame();

	const static std::string EditorAssets = Paths::Project + "/assets/editor/";

	constexpr static float TriaxisScreenposX = -1.80f;
	constexpr static float TriaxisScreenposY = -1.80f;


	void Initialize();
	void Update(EPlayer* Player, RWorld* World, RCamera* Camera);
	void Render(EPlayer* Player, RWorld* World, RCamera* Camera);
	void Terminate();

	void UpdateTriaxisGizmo();
	void CheckSelectionToOpenPanel(EPlayer* Player, RWorld* World, RCamera* Camera);
	bool CheckSelectionToGrabEntityArrows(RCamera* Camera);
	bool CheckSelectionToGrabEntityRotationGizmo(RCamera* Camera);
	void CheckSelectionToMoveEntity(RWorld* World, RCamera* Camera);
	void CheckSelectionToSelectRelatedEntity(RWorld* World, RCamera* Camera);

	void RenderTextOverlay(EPlayer* Player, RCamera* Camera);
	void RenderEventTriggers(RCamera* Camera, RWorld* World);
	void UpdateEntityControlArrows(REntityPanelContext* Panel);
	void RenderEntityControlArrows(REntityPanelContext* Panel, RWorld* World, RCamera* Camera);
	void RenderEntityRotationGizmo(REntityPanelContext* Panel, RWorld* World, RCamera* Camera);
	void UpdateEntityRotationGizmo(REntityPanelContext* Panel);
	void RenderEntityMeshNormals(REntityPanelContext* Panel);
	float GetGizmoScalingFactor(EEntity* Entity, float Min, float Max);
	void RenderWorldCells(RCamera* Camera, RWorld* World);
	void RenderLightbulbs(RCamera* Camera, RWorld* World);
	void StartDearImguiFrame();
	void EndDearImguiFrame();

	inline REditorContext* GetContext() { return REditorContext::Get(); }
}
