#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#define LOG_NONE   0
#define LOG_DEBUG   1
#define LOG_WARNING 2
#define LOG_ERROR   3


int init_logger(char* debug_file, char* warning_file, char* error_file);
int logmsgf(int level, const char* format, ...);

int debugf(const char* format, ...);
int warningf(const char* format, ...);
int errorf(const char* format, ...);

#endif // LOGGER_H