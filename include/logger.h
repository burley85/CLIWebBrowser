#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

struct logger {
    FILE* debug;
    FILE* warning;
    FILE* error;
} logger;

int init_logger(char* debug_file, char* warning_file, char* error_file);
int debugf(const char* format, ...);
int warningf(const char* format, ...);
int errorf(const char* format, ...);

#endif // LOGGER_H