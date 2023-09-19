#pragma once

#include "engine/core/core.h"
#include "editor_context.h"
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
	void Update(Player* player, T_World* world, Camera* camera);
	void Render(Player* player, T_World* world, Camera* camera);
	void Terminate();

	void UpdateTriaxisGizmo();
	void CheckSelectionToOpenPanel(Player* player, T_World* world, Camera* camera);
	bool CheckSelectionToGrabEntityArrows(Camera* camera);
	bool CheckSelectionToGrabEntityRotationGizmo(Camera* camera);
	void CheckSelectionToMoveEntity(T_World* world, Camera* camera);
	void CheckSelectionToSelectRelatedEntity(T_World* world, Camera* camera);

	void RenderTextOverlay(Player* player, Camera* camera);
	void RenderEventTriggers(Camera* camera, T_World* world);
	void UpdateEntityControlArrows(EntityPanelContext* panel);
	void RenderEntityControlArrows(EntityPanelContext* panel, T_World* world, Camera* camera);
	void RenderEntityRotationGizmo(EntityPanelContext* panel, T_World* world, Camera* camera);
	void UpdateEntityRotationGizmo(EntityPanelContext* panel);
	void RenderEntityMeshNormals(EntityPanelContext* panel);
	float GetGizmoScalingFactor(E_Entity* entity, float min, float max);
	void RenderWorldCells(Camera* camera, T_World* world);
	void RenderLightbulbs(Camera* camera, T_World* world);
	void StartDearImguiFrame();
	void EndDearImguiFrame();

	inline EditorContext* GetContext() { return EditorContext::Get(); }
}
