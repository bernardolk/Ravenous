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
	float GetGizmoScalingFactor(EEntity* entity, float min, float max);
	void RenderWorldCells(Camera* camera, World* world);
	void RenderLightbulbs(Camera* camera, World* world);
	void StartDearImguiFrame();
	void EndDearImguiFrame();

	inline EditorContext* GetContext() { return EditorContext::Get(); }
}
