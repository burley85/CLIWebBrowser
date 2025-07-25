#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "logger.h"

int init_logger(char* debug_file, char* warning_file, char* error_file) {
    memset(&logger, 0, sizeof(struct logger));

    if (error_file) {
        logger.error = fopen(error_file, "w");
        if (!logger.error) {
            perror("Failed to open error log file");
            return -1;
        }
    }
    else {
        logger.error = stderr;
    }

    if (warning_file) {
        logger.warning = fopen(warning_file, "w");
        if (!logger.warning) {
            perror("Failed to open warning log file");
            return -1;
        }
    }

    if (debug_file) {
        logger.debug = fopen(debug_file, "w");
        if (!logger.debug) {
            perror("Failed to open debug log file");
            return -1;
        }
    }

    return 0;
}

char* log_prefix(int level) {
    switch (level) {
        case LOG_DEBUG:
            return "DEBUG: ";
        case LOG_WARNING:
            return "WARNING: ";
        case LOG_ERROR:
            return "ERROR: ";
        default:
            return "";
    }
}

int logf(int level, const char* format, ...) {
    FILE* file = NULL;

    switch (level) {
        case LOG_DEBUG:
            file = logger.debug;
            break;
        case LOG_WARNING:
            file = logger.warning;
            break;
        case LOG_ERROR:
            file = logger.error;
            break;
        default:
            return 0;
    }

    if (file) {
        va_list args;
        va_start(args, format);
        fprintf(file, "%s", log_prefix(level));
        int result = vfprintf(file, format, args);
        va_end(args);
        fflush(file);
        return result;
    }
    return 0;
}

int debugf(const char* format, ...) {
    if (logger.debug) {
        va_list args;
        va_start(args, format);
        fprintf(logger.debug, "%s", log_prefix(LOG_DEBUG));
        int result = vfprintf(logger.debug, format, args);
        va_end(args);
        return result;
    }
    return 0;
}

int warningf(const char* format, ...) {
    if (logger.warning) {
        va_list args;
        va_start(args, format);
        fprintf(logger.warning, "%s", log_prefix(LOG_WARNING));
        int result = vfprintf(logger.warning, format, args);
        va_end(args);
        return result;
    }
    return 0;
}

int errorf(const char* format, ...) {
    if (logger.error) {
        va_list args;
        va_start(args, format);
        fprintf(logger.error, "%s", log_prefix(LOG_ERROR));
        int result = vfprintf(logger.error, format, args);
        va_end(args);
        return result;
    }
    return 0;
}