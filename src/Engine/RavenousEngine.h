#pragma once
#include "Core/Macros.h"

namespace RavenousEngine
{
	struct RFrameData
	{
		float duration = 0;
		float real_duration = 0;
		float last_frame_time = 0;
		int fps = 0;
		int fps_counter = 0;
		float sub_second_counter = 0;
		float time_step = 1;
	};

	struct REngineRuntimeState
	{
		DeclSingleton(REngineRuntimeState)
		
		RFrameData frame;
	};
	
	void Initialize();
	void StartFrame();
	float GetFrameDuration();
	RFrameData& GetFrame();
}