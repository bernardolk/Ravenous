#pragma once

#include "Engine/Core/Core.h"

#if PLATFORM == OS_WINDOWS
#include "Engine/Platform/PlatformLayerWin.h"
#endif

namespace Platform
{
	inline void Initialize()
	{
	#if PLATFORM == OS_WINDOWS
		WinPlatformInitialize();
	#endif
	}

	inline bool ListFilesInDir(string path, string filetype, vector<string>& out_files)
	{
	#if PLATFORM == OS_WINDOWS
		return WinListFiles(path, filetype, out_files);
	#endif
	}

	inline float GetCurrentTime()
	{
	#if PLATFORM == OS_WINDOWS
		return WinGetCurrentTime();
	#endif
	}
}