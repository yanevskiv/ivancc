// (Test) Status: 0
// A test file for <stdio.h> the standard streams.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// The three standard streams should be three streams.
// [C99:7.19.3p7]: stdin, stdout and stderr at program startup
static void Test_StdioStreams(void)
{
    FILE *in = stdin;
    FILE *out = stdout;
    FILE *err = stderr;

    _TEST_TRUE(in != NULL);
    _TEST_TRUE(out != NULL);
    _TEST_TRUE(err != NULL);
    _TEST_TRUE(in != out);
    _TEST_TRUE(in != err);
    _TEST_TRUE(out != err);
}

int main(void)
{
    Test_StdioStreams();
    return _Test_Status();
}
