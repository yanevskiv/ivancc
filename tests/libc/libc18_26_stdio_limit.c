// (Test) Status: 0
// A test file for <stdio.h> the longest conversion.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define LIMIT 4095

static char got[LIMIT + 1];

// One conversion should produce 4095 characters at least.
// [C99:7.19.6.1p15]: at least 4095 characters from one conversion
static void Test_StdioLimit(void)
{
    _TEST_INT(snprintf(NULL, 0, "%4095s", "end"), LIMIT);

    _TEST_INT(sprintf(got, "%4095s", "end"), LIMIT);
    _TEST_UINT(strspn(got, " "), LIMIT - 3);
    _TEST_STR(got + LIMIT - 3, "end");

    _TEST_INT(sprintf(got, "%-*c", LIMIT, 'a'), LIMIT);
    _TEST_INT(got[0], 'a');
    _TEST_UINT(strspn(got + 1, " "), LIMIT - 1);
    _TEST_UINT(strlen(got), LIMIT);
}

int main(void)
{
    Test_StdioLimit();
    return _Test_Status();
}
