#pragma once

#include "engine/core/core.h"
#include "editor_context.h"
#include "engine/rvn.h"


namespace Editor
{
	void StartDearImguiFrame();
	void EndDearImguiFrame();

	const static std::string EDITOR_ASSETS = Paths::Project + "/assets/editor/";

	constexpr static float TRIAXIS_SCREENPOS_X = -1.80f;
	constexpr static float TRIAXIS_SCREENPOS_Y = -1.80f;


	void Initialize();
	void Update(Player* player, World* world, Camera* camera);
	void Render(Player* player, World* world, Camera* camera);
	void Terminate();

	void UpdateTriaxisGizmo();
	void CheckSelectionToOpenPanel(Player* player, World* world, Camera* camera);
	bool CheckSelectionToGrabEntityArrows(Camera* camera);
	bool CheckSelectionToGrabEntityRotationGizmo(Camera* camera);
	void CheckSelectionToMoveEntity(World* world, Camera* camera);
	void CheckSelectionToSelectRelatedEntity(World* world, Camera* camera);

	void RenderTextOverlay(Player* player, Camera* camera);
	void RenderEventTriggers(Camera* camera, World* world);
	void UpdateEntityControlArrows(EntityPanelContext* panel);
	void RenderEntityControlArrows(EntityPanelContext* panel, World* world, Camera* camera);
	void RenderEntityRotationGizmo(EntityPanelContext* panel, World* world, Camera* camera);
	void UpdateEntityRotationGizmo(EntityPanelContext* panel);
	void RenderEntityMeshNormals(EntityPanelContext* panel);
	float GetGizmoScalingFactor(Entity* entity, float min, float max);
	void RenderWorldCells(Camera* camera, World* world);
	void RenderLightbulbs(Camera* camera, World* world);
	void StartDearImguiFrame();
	void EndDearImguiFrame();

	inline EditorContext* GetContext() { return EditorContext::Get(); }
}
