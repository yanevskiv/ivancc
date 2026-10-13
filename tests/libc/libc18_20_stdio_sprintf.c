// (Test) Status: 0
// A test file for <stdio.h> sprintf.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99 S7.19.6.6p2] sprintf should write a format's text and a null after it.
static void Test_StdioSprintf(void)
{
    char got[SMALL];

    memset(got, FILL, sizeof(got));
    _TEST_INT(sprintf(got, "plain"), 5);
    _TEST_MEM(got, "plain\0#", 7);

    memset(got, FILL, sizeof(got));
    _TEST_INT(sprintf(got, ""), 0);
    _TEST_MEM(got, "\0#", 2);

    _TEST_INT(sprintf(got, "100%%"), 4);
    _TEST_STR(got, "100%");
    _TEST_INT(sprintf(got, "%%%%"), 2);
    _TEST_STR(got, "%%");
    _TEST_INT(sprintf(got, "%s and %c", "abc", 'd'), 9);
    _TEST_STR(got, "abc and d");
    _TEST_INT(sprintf(got, "%s%s%s", "a", "", "b"), 2);
    _TEST_STR(got, "ab");
}

int main(void)
{
    Test_StdioSprintf();
    return _Test_Status();
}
