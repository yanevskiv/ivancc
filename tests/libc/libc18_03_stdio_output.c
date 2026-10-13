// (Test) Status: 0
// (Test) Output:
// | abc
// | fputs, puts
// | fwrite 0123456789
// | a line longer than the buffer it passes through
// | after the flush
// | atexit's
// (Test) Error:
// | unbuffered
// A test file for <stdio.h> output through a small buffer.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

static char outbuf[SMALL];

// Write a line as exit runs, after all that main wrote.
static void last(void)
{
    puts("atexit's");
}

// [C99 S7.19.3p3] Each output function should write through a smaller buffer.
static void Test_StdioOutput(void)
{
    int bad = 0;

    while (bad == _IOFBF || bad == _IOLBF || bad == _IONBF) {
        bad++;
    }
    _TEST_TRUE(setvbuf(stdout, NULL, bad, 0) != 0);
    _TEST_INT(setvbuf(stdout, outbuf, _IOFBF, sizeof(outbuf)), 0);
    setbuf(stderr, NULL);
    _TEST_INT(atexit(last), 0);

    _TEST_INT(putchar('a'), 'a');
    _TEST_INT(putc('b', stdout), 'b');
    _TEST_INT(fputc('c' + UCHAR_MAX + 1, stdout), 'c');
    _TEST_INT(fputc('\n', stdout), '\n');
    _TEST_TRUE(fputs("fputs, ", stdout) >= 0);
    _TEST_TRUE(puts("puts") >= 0);
    _TEST_UINT(fwrite("fwrite ", 1, 7, stdout), 7);
    _TEST_UINT(fwrite("0123456789", 5, 2, stdout), 2);
    _TEST_UINT(fwrite("x", 0, 3, stdout), 0);
    _TEST_UINT(fwrite("x", 1, 0, stdout), 0);
    _TEST_INT(putchar('\n'), '\n');
    _TEST_TRUE(fputs(LONGER, stdout) >= 0);
    _TEST_INT(fflush(stdout), 0);
    _TEST_TRUE(fputs("after the flush\n", stdout) >= 0);
    _TEST_INT(fflush(NULL), 0);

    _TEST_TRUE(fputs("unbuffered\n", stderr) >= 0);
    _TEST_INT(fflush(stderr), 0);
}

int main(void)
{
    Test_StdioOutput();
    return _Test_Status();
}
