#pragma once

namespace RavenousEngine
{
	struct RFrameData
	{
		float Duration = 0;
		float RealDuration = 0;
		float LastFrameTime = 0;
		int Fps = 0;
		int FpsCounter = 0;
		float SubSecondCounter = 0;
		float TimeStep = 1;
	};

	struct REngineRuntimeState
	{
		static REngineRuntimeState* Get()
		{
			static REngineRuntimeState Instance{};
			return &Instance;
		}
		
		RFrameData Frame;
	};

	void Initialize();
	void StartFrame();
	float GetFrameDuration();
	RFrameData& GetFrame();
}
