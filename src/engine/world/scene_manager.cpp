#include "scene_manager.h"
#include "scene.h"
#include "player.h"

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