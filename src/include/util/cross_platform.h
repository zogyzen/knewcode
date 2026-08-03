#pragma once

#include <string>
#include <cstdlib> // For std::getenv in Unix-like systems, and potentially other utilities.

#ifdef _WIN32 // Windows-specific code.
    #include <windows.h> // For GetModuleFileName on Windows.
#else // Unix-like systems (Linux, macOS)
    #include <unistd.h> // For readlink on Unix-like systems.
    #include <limits.h> // For PATH_MAX.
#endif // _WIN32

namespace KC
{
    class CrossPlatform
    {
    public:
        // 得到可执行文件路径
        static std::string GetExecutablePath()
        {
        #ifdef _WIN32 // Windows-specific code.
            char path[MAX_PATH];
            ::GetModuleFileNameA(NULL, path, sizeof(path));
            return std::string(path);
        #else // Unix-like systems (Linux, macOS)
            char path[PATH_MAX];
            ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
            if (count == -1)
                return std::string(); // Error handling.
            else
                return std::string(path, count); // Exclude null terminator.
        #endif // _WIN32
        }
    };
}
