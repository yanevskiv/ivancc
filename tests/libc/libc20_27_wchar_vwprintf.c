// (Test) Status: 0
// (Test) Output:
// | wprintf: abc 1
// | vwprintf: def 2
// A test file for <wchar.h> formatting an argument list.

// Include helpers.
#include "libc20_00_wchar_helpers.inc"

#define NAME "wide.tmp"

// Print the arguments after a wide format to stdout by vwprintf.
static int to_stdout(const wchar_t *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vwprintf(format, args);
    va_end(args);
    return n;
}

// Print the arguments after a wide format to a stream by vfwprintf.
static int to_stream(FILE *file, const wchar_t *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vfwprintf(file, format, args);
    va_end(args);
    return n;
}

// Print the arguments after a wide format into a wide string by vswprintf.
static int to_sized(wchar_t *str, size_t size, const wchar_t *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vswprintf(str, size, format, args);
    va_end(args);
    return n;
}

// Print a list's arguments twice, which vswprintf should leave to its caller.
static int to_both(wchar_t *first, wchar_t *second, const wchar_t *format, ...)
{
    int n;
    va_list args;
    va_list copy;

    va_start(args, format);
    va_copy(copy, args);
    n = vswprintf(first, SIZE, format, args);
    n += vswprintf(second, SIZE, format, copy);
    va_end(copy);
    va_end(args);
    return n;
}

// Each function that takes a list should convert its arguments.
// [C99:7.24.2.7]: vswprintf
static void Test_WcharVwprintf(void)
{
    FILE *file;
    wchar_t out[SIZE];
    wchar_t again[SIZE];

    _TEST_INT(to_sized(out, SIZE, L"%d|%s|%ls|%c", 12, "ab", L"cd", 'e'), 10);
    _TEST_MEM(out, L"12|ab|cd|e", WIDE(11));
    _TEST_TRUE(to_sized(out, 3, L"%d", 1234) < 0);
    _TEST_INT(to_both(out, again, L"%d %g", 7, 0.5), 10);
    _TEST_MEM(out, L"7 0.5", WIDE(6));
    _TEST_MEM(again, L"7 0.5", WIDE(6));

    file = fopen(NAME, "wb");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(to_stream(file, L"%ls %d\n", L"file", 3), 7);
    _TEST_TRUE(fwide(file, 0) > 0);
    _TEST_INT(fclose(file), 0);
    holds(NAME, "file 3\n", 7);
    _TEST_INT(remove(NAME), 0);
}

// wprintf and vwprintf should write to standard output.
// [C99:7.24.2.11]: wprintf
static void Test_WcharWprintf(void)
{
    _TEST_INT(fwide(stdout, 0), 0);
    _TEST_INT(wprintf(L"wprintf: %s %d\n", "abc", 1), 15);
    _TEST_TRUE(fwide(stdout, 0) > 0);
    _TEST_INT(to_stdout(L"vwprintf: %ls %d\n", L"def", 2), 16);
    _TEST_INT(fflush(stdout), 0);
    _TEST_TRUE(! ferror(stdout));
}

int main(void)
{
    Test_WcharVwprintf();
    Test_WcharWprintf();
    return _Test_Status();
}
