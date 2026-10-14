// (Test) Status: 0
// (Test) Output:
// | vprintf: abc 1
// | vfprintf: def 2
// A test file for <stdio.h> printing an argument list.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// Print the arguments after a format to stdout by vprintf.
static int to_stdout(const char *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vprintf(format, args);
    va_end(args);
    return n;
}

// Print the arguments after a format to a stream by vfprintf.
static int to_stream(FILE *file, const char *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vfprintf(file, format, args);
    va_end(args);
    return n;
}

// Print the arguments after a format into a string by vsprintf.
static int to_string(char *str, const char *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vsprintf(str, format, args);
    va_end(args);
    return n;
}

// Print the arguments after a format into a string of a size by vsnprintf.
static int to_sized(char *str, size_t size, const char *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vsnprintf(str, size, format, args);
    va_end(args);
    return n;
}

// [C99:7.19.6.8p2] Each function that takes a list should convert its arguments.
static void Test_StdioVprintf(void)
{
    char got[SMALL];

    _TEST_INT(to_stdout("vprintf: %s %c\n", "abc", '1'), 15);
    _TEST_INT(to_stream(stdout, "vfprintf: %s %c\n", "def", '2'), 16);
    _TEST_INT(fflush(stdout), 0);

    memset(got, FILL, sizeof(got));
    _TEST_INT(to_string(got, "%s %c", "ghi", '3'), 5);
    _TEST_MEM(got, "ghi 3\0#", 7);

    memset(got, FILL, sizeof(got));
    _TEST_INT(to_sized(got, 4, "%s %c", "jkl", '4'), 5);
    _TEST_MEM(got, "jkl\0#", 5);
    _TEST_INT(to_sized(NULL, 0, "%*s", SMALL, ""), SMALL);
}

int main(void)
{
    Test_StdioVprintf();
    return _Test_Status();
}
