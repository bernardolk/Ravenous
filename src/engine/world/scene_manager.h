#pragma once

#include "engine/core/core.h"

struct GlobalSceneInfo
{
	Scene* active_scene = nullptr;
	Camera* camera = nullptr;
	Camera* views[2];
	Player* player = nullptr;
	bool input_mode = false;
	std::string scene_name;
	bool tmp_unstuck_things = false;

	static GlobalSceneInfo* Get() { static GlobalSceneInfo instance; return &instance; }

	static void RefreshActiveScene();
	static Camera* GetGameCam();
	static Camera* GetEditorCam();

private:
	GlobalSceneInfo();
	GlobalSceneInfo(const GlobalSceneInfo& other) = delete;
};