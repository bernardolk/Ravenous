#include "RavenousEngine.h"

#include "Platform/Platform.h"

void RavenousEngine::Initialize()
{
	Platform::Initialize();
}

void RavenousEngine::StartFrame()
{
	float CurrentFrameTime = Platform::GetCurrentTime();
	auto& Frame = REngineRuntimeState::Get()->Frame;

	Frame.RealDuration = CurrentFrameTime - Frame.LastFrameTime;
	Frame.Duration = Frame.RealDuration * Frame.TimeStep;
	Frame.LastFrameTime = CurrentFrameTime;

	// @TODO: Can't remember why this is important...
	// forces framerate for simulation to be small
	if (Frame.Duration > 0.02f)
	{
		Frame.Duration = 0.02f;
	}

	Frame.SubSecondCounter += Frame.RealDuration;
	Frame.FpsCounter += 1;
	if (Frame.SubSecondCounter > 1)
	{
		Frame.Fps = Frame.FpsCounter;
		Frame.FpsCounter = 0;
		Frame.SubSecondCounter -= 1;
	}
}

float RavenousEngine::GetFrameDuration()
{
	return REngineRuntimeState::Get()->Frame.Duration;
}

RavenousEngine::RFrameData& RavenousEngine::GetFrame()
{
	return REngineRuntimeState::Get()->Frame;
}