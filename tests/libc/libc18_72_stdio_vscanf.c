// (Test) Status: 0
// (Test) Input:
// | first second
// A test file for <stdio.h> vfscanf, vscanf and vsscanf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:S7.19.6.14p2] vsscanf should read a string as sscanf does, by a list.
static int reads_string(const char *str, const char *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vsscanf(str, format, args);
    va_end(args);
    return n;
}

// [C99:S7.19.6.9p2] vfscanf should read a stream as fscanf does, by a list.
static int reads_stream(FILE *file, const char *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vfscanf(file, format, args);
    va_end(args);
    return n;
}

// [C99:S7.19.6.11p2] vscanf should read standard input as scanf does, by a list.
static int reads_input(const char *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vscanf(format, args);
    va_end(args);
    return n;
}

// [C99:S7.19.6.9p2] Each function taking a list should read as its fellow does.
static void Test_StdioVscanf(void)
{
    FILE *file;
    int n = NONE;
    char first[SMALL];
    char second[SMALL];

    _TEST_INT(reads_string("one two", "%s%s%n", first, second, &n), 2);
    _TEST_STR(first, "one");
    _TEST_STR(second, "two");
    _TEST_INT(n, 7);
    _TEST_INT(reads_string("", "%s", first), EOF);

    _TEST_INT(reads_input("%s%s", first, second), 2);
    _TEST_STR(first, "first");
    _TEST_STR(second, "second");
    _TEST_INT(reads_input(" %s", first), EOF);

    if (! _TEST_TRUE(put("file", "w", "three four"))) {
        return;
    }
    file = fopen("file", "r");
    if (! _TEST_TRUE(file != NULL)) {
        return;
    }
    _TEST_INT(reads_stream(file, "%s%s", first, second), 2);
    _TEST_STR(first, "three");
    _TEST_STR(second, "four");
    _TEST_INT(reads_stream(file, "%s", first), EOF);
    _TEST_INT(fclose(file), 0);
}

int main(void)
{
    Test_StdioVscanf();
    return _Test_Status();
}
