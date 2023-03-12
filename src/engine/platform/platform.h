// ridiculous "platform abstraction" just to express the idea. That's not how this is done at all.
#pragma once

enum SupportedPlatforms
{
	OS_WINDOWS = 0
};

constexpr static SupportedPlatforms Platform = OS_WINDOWS;

#if PLATFORM == OS_WINDOWS
#include <engine/platform/win_platform_layer.h>
#endif

inline bool OSListFiles(std::string path, std::string filetype, std::vector<std::string>& files)
{
#if PLATFORM == OS_WINDOWS
	return WinListFiles(path, filetype, files);
#endif
}
