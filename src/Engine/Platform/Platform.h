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

	inline bool ListFilesInDir(string Path, string Filetype, vector<string>& OutFiles)
	{
#if PLATFORM == OS_WINDOWS
		return WinListFiles(Path, Filetype, OutFiles);
#endif
	}

	inline float GetCurrentTime()
	{
#if PLATFORM == OS_WINDOWS
		return WinGetCurrentTime();
#endif
	}
}
