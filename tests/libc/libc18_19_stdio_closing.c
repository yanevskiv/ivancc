// (Test) Status: 0
// A test file for <stdio.h> closing a standard stream.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// A standard stream should close as any other does.
// [C99:7.19.5.1p2]: fclose, flushing and closing
static void Test_StdioClosing(void)
{
    _TEST_INT(fclose(stdin), 0);
}

int main(void)
{
    Test_StdioClosing();
    return _Test_Status();
}
