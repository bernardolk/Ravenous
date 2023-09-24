#include "RavenousEngine.h"

#include "Platform/Platform.h"

void RavenousEngine::Initialize()
{
	Platform::Initialize();
}

void RavenousEngine::StartFrame()
{
	float current_frame_time = Platform::GetCurrentTime();
	auto& frame = EngineRuntimeState::Get()->frame;
	
	frame.real_duration = current_frame_time - frame.last_frame_time;
	frame.duration = frame.real_duration * frame.time_step;
	frame.last_frame_time = current_frame_time;

	// @TODO: Can't remember why this is important...
	// forces framerate for simulation to be small
	if (frame.duration > 0.02f)
	{
		frame.duration = 0.02f;
	}

	frame.sub_second_counter += frame.real_duration;
	frame.fps_counter += 1;
	if (frame.sub_second_counter > 1)
	{
		frame.fps = frame.fps_counter;
		frame.fps_counter = 0;
		frame.sub_second_counter -= 1;
	}
}

float RavenousEngine::GetFrameDuration()
{
	return EngineRuntimeState::Get()->frame.duration;	
}

RavenousEngine::FrameData& RavenousEngine::GetFrame()
{
	return EngineRuntimeState::Get()->frame;	
}

RavenousEngine::EngineRuntimeState::EngineRuntimeState() = default;

