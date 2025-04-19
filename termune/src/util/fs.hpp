#pragma once

#include <string>
#include <cstdlib>
#include <sys/stat.h>
#include <stdexcept>

namespace fs
{
    // Returns the path to $HOME/.rlg327
    inline std::string rlg327_data_dir()
    {
        const char *home = getenv("HOME");
        if (!home)
            throw std::runtime_error("HOME environment variable not set");

        return std::string(home) + "/.rlg327";
    }

    // Ensures the directory exists, creates it if necessary
    inline void ensure_data_dir_exists()
    {
        std::string dir = rlg327_data_dir();

        struct stat st = {0};
        if (stat(dir.c_str(), &st) == -1)
        {
            if (mkdir(dir.c_str(), 0700) == -1)
                throw std::runtime_error("Failed to create ~/.rlg327");
        }
    }

    // Joins two paths with a /
    inline std::string join(const std::string &a, const std::string &b)
    {
        if (a.empty())
            return b;
        if (b.empty())
            return a;
        if (a.back() == '/')
            return a + b;
        else
            return a + "/" + b;
    }
}
