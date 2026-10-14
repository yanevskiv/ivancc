// (Test) Status: 0
// (Test) Error:
// | first: Numerical argument out of domain
// | Numerical result out of range
// | Invalid or incomplete multibyte or wide character
// A test file for <stdio.h> perror.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.10.4p2] perror should write its string and errno's text to stderr.
static void Test_StdioPerror(void)
{
    errno = EDOM;
    perror("first");
    errno = ERANGE;
    perror(NULL);
    errno = EILSEQ;
    perror("");
    _TEST_TRUE(! ferror(stderr));
    _TEST_INT(fflush(stderr), 0);
}

int main(void)
{
    Test_StdioPerror();
    return _Test_Status();
}
