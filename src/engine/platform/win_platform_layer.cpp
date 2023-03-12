#include "win_platform_layer.h"
#include "windows.h"

bool WIN_list_files(std::string path, std::string mask, std::vector<std::string>& files)
{
	auto h_find = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA ffd;
	std::string spec;
	std::stack<std::string> directories;

	directories.push(path);
	files.clear();

	while (!directories.empty())
	{
		path = directories.top();
		spec = path + "\\" + mask;
		directories.pop();

		h_find = FindFirstFile(spec.c_str(), &ffd);
		if (h_find == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		do
		{
			if (strcmp(ffd.cFileName, ".") != 0 &&
				strcmp(ffd.cFileName, "..") != 0)
			{
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					directories.push(path + "\\" + ffd.cFileName);
				}
				else
				{
					files.push_back(path + "/" + ffd.cFileName);
				}
			}
		} while (FindNextFile(h_find, &ffd) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES)
		{
			FindClose(h_find);
			return false;
		}

		FindClose(h_find);
		h_find = INVALID_HANDLE_VALUE;
	}

	return true;
}
