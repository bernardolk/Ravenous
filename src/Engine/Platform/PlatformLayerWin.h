#pragma once

#include "engine/core/core.h"

#define OS_WINDOWS_INCLUDED

bool WinListFiles(string Path, string Mask, vector<string>& Files);

void WinPlatformInitialize();

float WinGetCurrentTime();

int WinGetCurrentSeconds();

int WinGetCurrentMicroseconds();