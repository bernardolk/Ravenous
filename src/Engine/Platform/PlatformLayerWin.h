#pragma once

#include "engine/core/core.h"

#define OS_WINDOWS_INCLUDED

bool WinListFiles(std::string path, std::string mask, std::vector<std::string>& files);

void WinPlatformInitialize();

float WinGetCurrentTime();