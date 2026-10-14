// (Test) Status: 0
// (Test) Output:
// | printf: abc, d and 100%
// | fprintf:   right|left  |
// (Test) Error:
// | stderr: abc
// A test file for <stdio.h> printf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.6.3p2] printf should write to stdout as fprintf does.
static void Test_StdioPrintf(void)
{
    _TEST_INT(printf("printf: %s, %c and 100%%\n", "abc", 'd'), 24);
    _TEST_INT(printf(""), 0);
    _TEST_INT(fprintf(stdout, "fprintf: %7s|%-6s|\n", "right", "left"), 25);
    _TEST_INT(fprintf(stderr, "stderr: %s\n", "abc"), 12);
    _TEST_TRUE(! ferror(stdout));
    _TEST_TRUE(! ferror(stderr));
    _TEST_INT(fflush(stdout), 0);
}

int main(void)
{
    Test_StdioPrintf();
    return _Test_Status();
}
