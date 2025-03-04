#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifndef LOG_FILE
#define LOG_FILE "log.txt"
#endif

/// Append a formatted string to the specified log file
#define LOG(format, ...)                                                      \
    do                                                                        \
    {                                                                         \
        FILE *log_file = fopen(LOG_FILE, "a");                                \
        if (log_file)                                                         \
        {                                                                     \
            time_t now = time(NULL);                                          \
            struct tm *t = localtime(&now);                                   \
            fprintf(log_file, "[%04d-%02d-%02d %02d:%02d:%02d] " format "\n", \
                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,             \
                    t->tm_hour, t->tm_min, t->tm_sec, ##__VA_ARGS__);         \
            fclose(log_file);                                                 \
        }                                                                     \
    } while (0)

/// Clear log file by opening in write and immediately closing
#define LOG_CLEAR()                            \
    do                                         \
    {                                          \
        FILE *log_file = fopen(LOG_FILE, "w"); \
        if (log_file)                          \
            fclose(log_file);                  \
    } while (0)