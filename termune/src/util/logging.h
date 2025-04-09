#pragma once

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <string>
#include <mutex>
#include <cstdarg>
#include <cstdio>

#include <iostream>

#ifndef LOG_FILE
#define LOG_FILE stderr
#endif

// Let user just -DLOG_FILE=stdout or stderr or a filename
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

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

    inline FILE *openLogStream()
    {
        static std::string target = TOSTRING(LOG_FILE);
        if (target == "stdout")
            return stdout;
        if (target == "stderr" || target == "stdlog")
            return stderr;

        return fopen(target.c_str(), "a");
    }

    inline void write(const char *format, ...)
    {

        std::lock_guard<std::mutex> lock(log_mutex());

        FILE *log_file = openLogStream();
        if (!log_file)
            return;

        std::fprintf(log_file, "%s ", timestamp().c_str());

        va_list args;
        va_start(args, format);
        std::vfprintf(log_file, format, args);
        va_end(args);

        std::fprintf(log_file, "\n");

        if (log_file != stdout && log_file != stderr)
            fclose(log_file);
    }

    inline void clear()
    {
        std::lock_guard<std::mutex> lock(log_mutex());

        static std::string target = TOSTRING(LOG_FILE);
        if (target == "stdout" || target == "stderr" || target == "stdlog")
            return; // Don't clear standard streams

        FILE *log_file = fopen(target.c_str(), "w");
        if (log_file)
            fclose(log_file);
    }
}
