#pragma once

#include "Engine/Core/Core.h"

#if PLATFORM == OS_WINDOWS
	#include "Engine/Platform/PlatformLayerWin.h"
#endif

namespace Platform
{
	
#if PLATFORM == OS_WINDOWS
	inline void Initialize()
	{
		WinPlatformInitialize();
	}

	inline bool ListFilesInDir(string Path, string Filetype, vector<string>& OutFiles)
	{
		return WinListFiles(Path, Filetype, OutFiles);
	}

	inline float GetCurrentTime()
	{
		return WinGetCurrentTime();
	}
	
	inline int GetCurrentSeconds()
	{
		return WinGetCurrentSeconds();
	}

	inline int GetCurrentMicroseconds()
	{
		return WinGetCurrentMicroseconds();
	}
#endif
	
}
