#include "scene_manager.h"
#include "engine/world/scene.h"
#include "game/entities/player.h"
#include "engine/camera/camera.h"

Camera* GlobalSceneInfo::GetGameCam()
{
	auto* GSI = Get();
	return GSI->views[GameCam];
}

Camera* GlobalSceneInfo::GetEditorCam()
{
	auto* GSI = Get();
	return GSI->views[EditorCam];
}
GlobalSceneInfo::GlobalSceneInfo()
{
	views[EditorCam] = new Camera();
	views[GameCam] = new Camera();

	camera = views[EditorCam];
}
