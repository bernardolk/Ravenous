#include "PlatformLayerWin.h"
#include <windows.h>
#include <stack>
#include "GlWindow.h"

bool WinListFiles(string Path, string Mask, vector<string>& Files)
{
	auto HFindFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA FindData;
	string Spec;
	std::stack<string> Directories;

	Directories.push(Path);
	Files.clear();

	while (!Directories.empty())
	{
		Path = Directories.top();
		Spec = Path + "\\" + Mask;
		Directories.pop();

		HFindFile = FindFirstFile(Spec.c_str(), &FindData);
		if (HFindFile == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		do
		{
			if (strcmp(FindData.cFileName, ".") != 0 &&
				strcmp(FindData.cFileName, "..") != 0)
			{
				if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					Directories.push(Path + "\\" + FindData.cFileName);
				}
				else
				{
					Files.push_back(Path + "/" + FindData.cFileName);
				}
			}
		} while (FindNextFile(HFindFile, &FindData) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES)
		{
			FindClose(HFindFile);
			return false;
		}

		FindClose(HFindFile);
		HFindFile = INVALID_HANDLE_VALUE;
	}

	return true;
}

void WinPlatformInitialize()
{
	SetupGLFW();
	SetupGL();
}

float WinGetCurrentTime()
{
	return glfwGetTime();
}
