// C source file for diagnostic messages.

// Take every include from the module's header.
#include "util/console/log.h"

// The word each severity prints as.
static const char *const Log_SeverityNames[LOG_SEVERITY_COUNT] = {
    [LOG_SEVERITY_ERROR]   = "error",
    [LOG_SEVERITY_WARNING] = "warning",
    [LOG_SEVERITY_INFO]    = "info",
    [LOG_SEVERITY_DEBUG]   = "debug"
};

// The name messages are reported under.
static const char *Log_ProgramName = LOG_PROGRAM_DEFAULT;

// The function that maps a line to its origin.
static Log_LineLocator Log_CurLineLocator;

// Report messages under the last component of a program path.
void Log_SetProgramName(const char *name)
{
    const char *slash = strrchr(name, '/');

    Log_ProgramName = slash ? slash + 1 : name;
}

// Map lines to their origin through locator.
void Log_SetLineLocator(Log_LineLocator locator)
{
    Log_CurLineLocator = locator;
}

// Print an error and exit.
void Log_ShowError(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    Log_ShowVa(LOG_SEVERITY_ERROR, LOG_LINE_NONE, fmt, ap);
    va_end(ap);
    exit(1);
}

// Print a warning.
void Log_ShowWarning(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    Log_ShowVa(LOG_SEVERITY_WARNING, LOG_LINE_NONE, fmt, ap);
    va_end(ap);
}

// Print a report.
void Log_ShowInfo(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    Log_ShowVa(LOG_SEVERITY_INFO, LOG_LINE_NONE, fmt, ap);
    va_end(ap);
}

// Print a debugging trace.
void Log_ShowDebug(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    Log_ShowVa(LOG_SEVERITY_DEBUG, LOG_LINE_NONE, fmt, ap);
    va_end(ap);
}

// Print one message at a line.
void Log_Show(Log_Severity severity, uint32_t line, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    Log_ShowVa(severity, line, fmt, ap);
    va_end(ap);
}

// Print one message at a line from an argument list.
void Log_ShowVa(Log_Severity severity, uint32_t line, const char *fmt, va_list ap)
{
    uint32_t source = line;

    if (line == LOG_LINE_NONE) {
        fprintf(stderr, "%s", Log_ProgramName);
    } else if (Log_CurLineLocator) {
        const char *path = Log_CurLineLocator(line, &source);
        fprintf(stderr, "%s:%u", path, source);
    } else {
        fprintf(stderr, "%s: line %u", Log_ProgramName, line);
    }
    fprintf(stderr, ": %s: ", Log_SeverityNames[severity]);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}
