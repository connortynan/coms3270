#pragma once

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <string>
#include <mutex>
#include <cstdarg>

#ifndef LOG_FILE
#define LOG_FILE "out.log"
#endif

namespace Log
{
    inline std::mutex &log_mutex()
    {
        static std::mutex m;
        return m;
    }

    inline std::string timestamp()
    {
        std::ostringstream oss;
        std::time_t now = std::time(nullptr);
        std::tm tm = *std::localtime(&now);
        oss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "]";
        return oss.str();
    }

    inline void write(const char *format, ...)
    {
        std::lock_guard<std::mutex> lock(log_mutex());

        FILE *log_file = fopen(LOG_FILE, "a");
        if (!log_file)
            return;

        std::fprintf(log_file, "%s ", timestamp().c_str());

        va_list args;
        va_start(args, format);
        std::vfprintf(log_file, format, args);
        va_end(args);

        std::fprintf(log_file, "\n");
        fclose(log_file);
    }

    inline void clear()
    {
        std::lock_guard<std::mutex> lock(log_mutex());
        FILE *log_file = fopen(LOG_FILE, "w");
        if (log_file)
            fclose(log_file);
    }

} // namespace Log
