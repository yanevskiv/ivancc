// C header file for diagnostic messages.

#ifndef LOG_H
#define LOG_H

// Standard headers.
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Program name a message carries before Log_SetProgramName.
#define LOG_PROGRAM_DEFAULT "ivancc"

// Line of a message that names no line.
#define LOG_LINE_NONE 0

// Map a line of the compiled text to its file and source line.
typedef const char *(*Log_LineLocator)(uint32_t line, uint32_t *source);

// Kinds of message.
typedef enum Log_Severity Log_Severity;
enum Log_Severity {
    LOG_SEVERITY_ERROR,
    LOG_SEVERITY_WARNING,
    LOG_SEVERITY_INFO,
    LOG_SEVERITY_DEBUG,
    LOG_SEVERITY_COUNT
};

// Setup
void Log_SetProgramName(const char *name);
void Log_SetLineLocator(Log_LineLocator locator);

// Messages
void Log_ShowError(const char *fmt, ...);
void Log_ShowWarning(const char *fmt, ...);
void Log_ShowInfo(const char *fmt, ...);
void Log_ShowDebug(const char *fmt, ...);
void Log_Show(Log_Severity severity, uint32_t line, const char *fmt, ...);
void Log_ShowVa(Log_Severity severity, uint32_t line, const char *fmt, va_list ap);

#endif // LOG_H
