// C header file for diagnostics.

#ifndef LOG_H
#define LOG_H

#include <stdlib.h>

#include "util/file.h"

// Print a diagnostic and exit.
#define Log_ShowError(...)                            \
    do {                                              \
        File_Print(File_Err(), "cc: error: ");        \
        File_Print(File_Err(), __VA_ARGS__);          \
        File_Print(File_Err(), "\n");                 \
        exit(1);                                      \
    } while (0)

// Print a diagnostic naming the source line it came from and exit.
#define Log_ShowErrorAt(line, ...)                             \
    do {                                                       \
        File_Print(File_Err(), "cc: error: line %d: ", (line)); \
        File_Print(File_Err(), __VA_ARGS__);                   \
        File_Print(File_Err(), "\n");                          \
        exit(1);                                               \
    } while (0)

// Print a warning diagnostic and continue.
#define Log_ShowWarning(...)                          \
    do {                                              \
        File_Print(File_Err(), "cc: warning: ");      \
        File_Print(File_Err(), __VA_ARGS__);          \
        File_Print(File_Err(), "\n");                 \
    } while (0)

#endif // LOG_H
