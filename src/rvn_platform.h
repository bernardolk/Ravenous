// ridiculous "platform abstraction" just to express the idea. That's not how this is done at all.
enum SupportedPlatforms {
   OS_WINDOWS = 0
};

const static SupportedPlatforms PLATFORM = OS_WINDOWS;

#if PLATFORM == OS_WINDOWS
   #include <rvn_win_layer.h>
#endif

bool OS_list_files(string path, string filetype, std::vector<string>& files)
{
   #if PLATFORM == OS_WINDOWS
      return WIN_list_files(path, filetype, files);
   #endif
}