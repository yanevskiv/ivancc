// (Test) Status: 0
// A test file for <stdio.h> snprintf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99/S7.19.6.5p2] snprintf should keep n - 1 characters and count them all.
static void Test_StdioSnprintf(void)
{
    char got[SMALL];

    _TEST_INT(snprintf(NULL, 0, "%s", "abcdef"), 6);

    memset(got, FILL, sizeof(got));
    _TEST_INT(snprintf(got, 0, "%s", "abcdef"), 6);
    _TEST_MEM(got, "##", 2);

    _TEST_INT(snprintf(got, 1, "%s", "abcdef"), 6);
    _TEST_MEM(got, "\0#", 2);

    memset(got, FILL, sizeof(got));
    _TEST_INT(snprintf(got, 4, "%s", "abcdef"), 6);
    _TEST_MEM(got, "abc\0#", 5);

    memset(got, FILL, sizeof(got));
    _TEST_INT(snprintf(got, 6, "ab%scd%c", "", 'e'), 5);
    _TEST_MEM(got, "abcde\0#", 7);

    memset(got, FILL, sizeof(got));
    _TEST_INT(snprintf(got, 5, "ab%scd%c", "", 'e'), 5);
    _TEST_MEM(got, "abcd\0#", 6);

    memset(got, FILL, sizeof(got));
    _TEST_INT(snprintf(got, sizeof(got), "%s", "abcdef"), 6);
    _TEST_MEM(got, "abcdef\0#", 8);
}

int main(void)
{
    Test_StdioSnprintf();
    return _Test_Status();
}
