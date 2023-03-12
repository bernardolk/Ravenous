#include "scene_manager.h"
#include "engine/world/scene.h"
#include "game/entities/player.h"
#include "engine/camera/camera.h"

void GlobalSceneInfo::RefreshActiveScene()
{
	auto* GSI = Get();
	if(GSI->active_scene == nullptr)
	{
		GSI->active_scene = new Scene();
		GSI->player = Player::Get();
	}
	else
	{
		*GSI->active_scene = Scene{};
		GSI->player = Player::ResetPlayer();
	}
}


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
